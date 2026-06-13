#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_DRIVERS 32
#define MAX_LINE 128

enum command_type {
    CMD_TASK = 1,
    CMD_STATUS,
    CMD_EXIT
};

enum answer_type {
    ANSWER_OK = 1,
    ANSWER_AVAILABLE,
    ANSWER_BUSY,
    ANSWER_ERROR
};

struct command {
    int type;
    int timer;
};

struct answer {
    int type;
    int timer;
};

struct driver {
    pid_t pid;
    int to_driver;
    int from_driver;
};

static volatile sig_atomic_t g_timer_done = 0;
static volatile sig_atomic_t g_need_stop = 0;

static void timer_handler(int signo)
{
    (void)signo;
    g_timer_done = 1;
}

static void stop_handler(int signo)
{
    (void)signo;
    g_need_stop = 1;
}

static int write_full(int fd, const void *buffer, size_t size)
{
    const char *ptr = buffer;

    while (size > 0) {
        ssize_t written = write(fd, ptr, size);

        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        ptr += written;
        size -= (size_t)written;
    }

    return 0;
}

static int read_full(int fd, void *buffer, size_t size)
{
    char *ptr = buffer;

    while (size > 0) {
        ssize_t bytes_read = read(fd, ptr, size);

        if (bytes_read == 0) {
            return 0;
        }

        if (bytes_read == -1) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        ptr += bytes_read;
        size -= (size_t)bytes_read;
    }

    return 1;
}

static int seconds_left(time_t busy_until)
{
    time_t now = time(NULL);

    if (busy_until <= now) {
        return 0;
    }

    return (int)(busy_until - now);
}

static void send_answer(int fd, int type, int timer)
{
    struct answer answer;

    answer.type = type;
    answer.timer = timer;

    if (write_full(fd, &answer, sizeof(answer)) == -1) {
        perror("write");
    }
}

static void setup_driver_signals(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = stop_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

static void driver_loop(int read_fd, int write_fd)
{
    int is_busy = 0;
    time_t busy_until = 0;
    struct pollfd pfd;

    setup_driver_signals();

    pfd.fd = read_fd;
    pfd.events = POLLIN;

    while (!g_need_stop) {
        int ret;

        if (g_timer_done) {
            g_timer_done = 0;
            is_busy = 0;
            busy_until = 0;
        }

        ret = poll(&pfd, 1, -1);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("poll");
            break;
        }

        if (pfd.revents & POLLHUP) {
            break;
        }

        if (pfd.revents & POLLIN) {
            struct command command;
            int read_status = read_full(read_fd, &command, sizeof(command));

            if (read_status <= 0) {
                break;
            }

            if (g_timer_done) {
                g_timer_done = 0;
                is_busy = 0;
                busy_until = 0;
            }

            if (command.type == CMD_TASK) {
                if (is_busy) {
                    send_answer(write_fd, ANSWER_BUSY, seconds_left(busy_until));
                    continue;
                }

                is_busy = 1;
                busy_until = time(NULL) + command.timer;
                alarm((unsigned int)command.timer);
                send_answer(write_fd, ANSWER_OK, command.timer);
            } else if (command.type == CMD_STATUS) {
                if (is_busy) {
                    send_answer(write_fd, ANSWER_BUSY, seconds_left(busy_until));
                } else {
                    send_answer(write_fd, ANSWER_AVAILABLE, 0);
                }
            } else if (command.type == CMD_EXIT) {
                break;
            } else {
                send_answer(write_fd, ANSWER_ERROR, 0);
            }
        }
    }

    close(read_fd);
    close(write_fd);
    exit(0);
}

static struct driver *find_driver(struct driver *drivers, int count, pid_t pid)
{
    for (int i = 0; i < count; ++i) {
        if (drivers[i].pid == pid) {
            return &drivers[i];
        }
    }

    return NULL;
}

static int ask_driver(struct driver *driver, const struct command *command, struct answer *answer)
{
    int read_status;

    if (write_full(driver->to_driver, command, sizeof(*command)) == -1) {
        perror("write");
        return -1;
    }

    read_status = read_full(driver->from_driver, answer, sizeof(*answer));
    if (read_status <= 0) {
        perror("read");
        return -1;
    }

    return 0;
}

static void print_answer(const struct answer *answer)
{
    if (answer->type == ANSWER_OK) {
        printf("Task accepted\n");
    } else if (answer->type == ANSWER_AVAILABLE) {
        printf("Available\n");
    } else if (answer->type == ANSWER_BUSY) {
        printf("Busy %d\n", answer->timer);
    } else {
        printf("Error\n");
    }
}

static int create_driver(struct driver *drivers, int *count)
{
    int to_driver[2];
    int from_driver[2];
    pid_t pid;

    if (*count >= MAX_DRIVERS) {
        printf("Too many drivers\n");
        return -1;
    }

    if (pipe(to_driver) == -1 || pipe(from_driver) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(to_driver[0]);
        close(to_driver[1]);
        close(from_driver[0]);
        close(from_driver[1]);
        return -1;
    }

    if (pid == 0) {
        close(to_driver[1]);
        close(from_driver[0]);
        driver_loop(to_driver[0], from_driver[1]);
    }

    close(to_driver[0]);
    close(from_driver[1]);

    drivers[*count].pid = pid;
    drivers[*count].to_driver = to_driver[1];
    drivers[*count].from_driver = from_driver[0];
    ++(*count);

    printf("Created driver %d\n", pid);
    return 0;
}

static void send_task(struct driver *drivers, int count, pid_t pid, int timer)
{
    struct driver *driver = find_driver(drivers, count, pid);
    struct command command;
    struct answer answer;

    if (driver == NULL) {
        printf("Driver not found\n");
        return;
    }

    command.type = CMD_TASK;
    command.timer = timer;

    if (ask_driver(driver, &command, &answer) == 0) {
        print_answer(&answer);
    }
}

static void get_status(struct driver *drivers, int count, pid_t pid)
{
    struct driver *driver = find_driver(drivers, count, pid);
    struct command command;
    struct answer answer;

    if (driver == NULL) {
        printf("Driver not found\n");
        return;
    }

    command.type = CMD_STATUS;
    command.timer = 0;

    if (ask_driver(driver, &command, &answer) == 0) {
        print_answer(&answer);
    }
}

static void get_drivers(struct driver *drivers, int count)
{
    struct command command;

    command.type = CMD_STATUS;
    command.timer = 0;

    for (int i = 0; i < count; ++i) {
        struct answer answer;

        printf("%d: ", drivers[i].pid);
        if (ask_driver(&drivers[i], &command, &answer) == 0) {
            print_answer(&answer);
        }
    }
}

static void stop_drivers(struct driver *drivers, int count)
{
    struct command command;

    command.type = CMD_EXIT;
    command.timer = 0;

    for (int i = 0; i < count; ++i) {
        write_full(drivers[i].to_driver, &command, sizeof(command));
        close(drivers[i].to_driver);
        close(drivers[i].from_driver);
        waitpid(drivers[i].pid, NULL, 0);
    }
}

static void print_help(void)
{
    printf("Commands:\n");
    printf("create_driver\n");
    printf("send_task <pid> <task_timer>\n");
    printf("get_status <pid>\n");
    printf("get_drivers\n");
    printf("exit\n");
}

int main(void)
{
    struct driver drivers[MAX_DRIVERS];
    int drivers_count = 0;
    char line[MAX_LINE];

    print_help();

    while (1) {
        char command[MAX_LINE];
        int pid;
        int timer;

        printf("taxi> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        if (sscanf(line, "%127s", command) != 1) {
            continue;
        }

        if (strcmp(command, "create_driver") == 0) {
            create_driver(drivers, &drivers_count);
        } else if (strcmp(command, "send_task") == 0) {
            if (sscanf(line, "%*s %d %d", &pid, &timer) != 2 || timer <= 0) {
                printf("Usage: send_task <pid> <task_timer>\n");
                continue;
            }

            send_task(drivers, drivers_count, (pid_t)pid, timer);
        } else if (strcmp(command, "get_status") == 0) {
            if (sscanf(line, "%*s %d", &pid) != 1) {
                printf("Usage: get_status <pid>\n");
                continue;
            }

            get_status(drivers, drivers_count, (pid_t)pid);
        } else if (strcmp(command, "get_drivers") == 0) {
            get_drivers(drivers, drivers_count);
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Unknown command\n");
        }
    }

    stop_drivers(drivers, drivers_count);
    return 0;
}
