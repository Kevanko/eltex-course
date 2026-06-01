#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LISTENER_PORT 7777
#define WORKER_PORT_BASE 12520
#define WORKER_COUNT 3
#define BACKLOG 16

typedef struct {
    int id;
    int port;
    int listen_fd;
    int busy;
} Worker;

static Worker workers[WORKER_COUNT];
static pthread_mutex_t workers_mutex = PTHREAD_MUTEX_INITIALIZER;

static void make_time_response(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;

    localtime_r(&now, &tm_now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S\n", &tm_now);
}

static int create_listener(int port)
{
    int listen_fd;
    int opt = 1;
    struct sockaddr_in addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("socket");
        return -1;
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(listen_fd);
        return -1;
    }

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

static void serve_time(int client_fd)
{
    char request[64];
    char response[64];
    ssize_t bytes_read = read(client_fd, request, sizeof(request) - 1);

    if (bytes_read > 0) {
        request[bytes_read] = '\0';
        make_time_response(response, sizeof(response));
        if (write(client_fd, response, strlen(response)) == -1) {
            perror("write");
        }
    } else if (bytes_read == -1) {
        perror("read");
    }
}

static void *worker_main(void *arg)
{
    Worker *worker = arg;

    while (1) {
        int client_fd = accept(worker->listen_fd, NULL, NULL);

        if (client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
            }
            continue;
        }

        serve_time(client_fd);
        close(client_fd);

        pthread_mutex_lock(&workers_mutex);
        worker->busy = 0;
        pthread_mutex_unlock(&workers_mutex);

        printf("worker %d is free\n", worker->id);
    }

    return NULL;
}

static int reserve_worker(void)
{
    int port = -1;

    pthread_mutex_lock(&workers_mutex);
    for (int i = 0; i < WORKER_COUNT; ++i) {
        if (!workers[i].busy) {
            workers[i].busy = 1;
            port = workers[i].port;
            break;
        }
    }
    pthread_mutex_unlock(&workers_mutex);

    return port;
}

static int start_workers(void)
{
    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_t thread;

        workers[i].id = i + 1;
        workers[i].port = WORKER_PORT_BASE + i;
        workers[i].busy = 0;
        workers[i].listen_fd = create_listener(workers[i].port);
        if (workers[i].listen_fd == -1) {
            return -1;
        }

        if (pthread_create(&thread, NULL, worker_main, &workers[i]) != 0) {
            perror("pthread_create");
            return -1;
        }

        pthread_detach(thread);
    }

    return 0;
}

int main(void)
{
    int listener_fd;

    if (start_workers() == -1) {
        return 1;
    }

    listener_fd = create_listener(LISTENER_PORT);
    if (listener_fd == -1) {
        return 1;
    }

    printf("Pool listener started on port %d\n", LISTENER_PORT);

    while (1) {
        int client_fd;
        int port;
        char response[64];
        char request[64];

        client_fd = accept(listener_fd, NULL, NULL);
        if (client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
            }
            continue;
        }

        if (read(client_fd, request, sizeof(request) - 1) == -1) {
            perror("read");
            close(client_fd);
            continue;
        }

        port = reserve_worker();
        if (port == -1) {
            snprintf(response, sizeof(response), "BUSY\n");
        } else {
            snprintf(response, sizeof(response), "PORT %d\n", port);
        }

        if (write(client_fd, response, strlen(response)) == -1) {
            perror("write");
        }

        close(client_fd);
    }

    close(listener_fd);
    return 0;
}
