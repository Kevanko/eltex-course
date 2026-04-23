#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "chat.h"

static volatile sig_atomic_t g_running = 1;
static int g_shm_fd = -1;
static ChatRoom *g_room = NULL;

static void sleep_briefly(void)
{
    const struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 100000000
    };

    nanosleep(&delay, NULL);
}

static void handle_signal(int signo)
{
    (void)signo;
    g_running = 0;
}

static int room_find_user(const ChatRoom *room, pid_t pid)
{
    for (int i = 0; i < room->user_count; ++i) {
        if (room->users[i].pid == pid) {
            return i;
        }
    }

    return -1;
}

static void room_add_message_locked(ChatRoom *room, const char *text)
{
    if (room->message_count < CHAT_MAX_MESSAGES) {
        snprintf(room->messages[room->message_count],
                 sizeof(room->messages[room->message_count]), "%s", text);
        ++room->message_count;
    } else {
        memmove(room->messages, room->messages + 1,
                sizeof(room->messages[0]) * (CHAT_MAX_MESSAGES - 1));
        snprintf(room->messages[CHAT_MAX_MESSAGES - 1],
                 sizeof(room->messages[CHAT_MAX_MESSAGES - 1]), "%s", text);
    }
}

static void room_add_user_locked(ChatRoom *room, pid_t pid, const char *name)
{
    if (room->user_count >= CHAT_MAX_CLIENTS) {
        return;
    }

    room->users[room->user_count].pid = pid;
    snprintf(room->users[room->user_count].name,
             sizeof(room->users[room->user_count].name), "%s", name);
    ++room->user_count;
}

static void room_remove_user_locked(ChatRoom *room, int index)
{
    if (index < 0 || index >= room->user_count) {
        return;
    }

    if (index < room->user_count - 1) {
        memmove(&room->users[index], &room->users[index + 1],
                sizeof(room->users[0]) * (size_t)(room->user_count - index - 1));
    }

    --room->user_count;
    room->users[room->user_count].pid = 0;
    room->users[room->user_count].name[0] = '\0';
}

static int room_pop_command_locked(ChatRoom *room, ChatRequest *request)
{
    if (room->command_count == 0) {
        return -1;
    }

    *request = room->commands[room->command_head];
    room->command_head = (room->command_head + 1) % CHAT_MAX_COMMANDS;
    --room->command_count;
    return 0;
}

static void room_process_join_locked(ChatRoom *room, const ChatRequest *request)
{
    char line[CHAT_LINE_LENGTH];

    if (room_find_user(room, request->pid) == -1) {
        room_add_user_locked(room, request->pid, request->name);
    }

    snprintf(line, sizeof(line), "%s joined the chat", request->name);
    room_add_message_locked(room, line);
    ++room->version;
    printf("%s\n", line);
}

static void room_process_text_locked(ChatRoom *room, const ChatRequest *request)
{
    char line[CHAT_LINE_LENGTH];

    if (request->text[0] == '\0') {
        return;
    }

    snprintf(line, sizeof(line), "%s: %s", request->name, request->text);
    room_add_message_locked(room, line);
    ++room->version;
    printf("%s\n", line);
}

static void room_process_leave_locked(ChatRoom *room, const ChatRequest *request)
{
    char line[CHAT_LINE_LENGTH];
    int index = room_find_user(room, request->pid);

    if (index == -1) {
        return;
    }

    snprintf(line, sizeof(line), "%s left the chat", room->users[index].name);
    room_remove_user_locked(room, index);
    room_add_message_locked(room, line);
    ++room->version;
    printf("%s\n", line);
}

static void room_process_command_locked(ChatRoom *room, const ChatRequest *request)
{
    if (request->command == CHAT_CMD_JOIN) {
        room_process_join_locked(room, request);
        return;
    }

    if (request->command == CHAT_CMD_TEXT) {
        room_process_text_locked(room, request);
        return;
    }

    if (request->command == CHAT_CMD_LEAVE) {
        room_process_leave_locked(room, request);
    }
}

static int room_create(void)
{
    g_shm_fd = shm_open(CHAT_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (g_shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    if (ftruncate(g_shm_fd, sizeof(*g_room)) == -1) {
        perror("ftruncate");
        return -1;
    }

    g_room = mmap(NULL, sizeof(*g_room), PROT_READ | PROT_WRITE, MAP_SHARED, g_shm_fd, 0);
    if (g_room == MAP_FAILED) {
        perror("mmap");
        g_room = NULL;
        return -1;
    }

    memset(g_room, 0, sizeof(*g_room));
    if (sem_init(&g_room->lock, 1, 1) == -1) {
        perror("sem_init");
        return -1;
    }

    return 0;
}

static void room_destroy(void)
{
    if (g_room != NULL) {
        sem_destroy(&g_room->lock);
        munmap(g_room, sizeof(*g_room));
        g_room = NULL;
    }

    if (g_shm_fd != -1) {
        close(g_shm_fd);
        g_shm_fd = -1;
    }

    shm_unlink(CHAT_SHM_NAME);
}

int main(void)
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    shm_unlink(CHAT_SHM_NAME);
    if (room_create() == -1) {
        room_destroy();
        return 1;
    }

    printf("Chat server started. Press Ctrl+C to stop.\n");

    while (g_running) {
        ChatRequest request = {0};
        bool processed = false;

        if (sem_wait(&g_room->lock) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("sem_wait");
            break;
        }

        if (room_pop_command_locked(g_room, &request) == 0) {
            room_process_command_locked(g_room, &request);
            processed = true;
        }

        sem_post(&g_room->lock);

        if (!processed) {
            sleep_briefly();
        }
    }

    room_destroy();

    return 0;
}
