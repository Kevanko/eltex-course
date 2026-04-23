#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chat.h"

typedef struct {
    pid_t pid;
    mqd_t queue;
    char name[MAX_NAME];
} ClientInfo;

static mqd_t g_server_queue = (mqd_t)-1;
static const char USERS_RESET_MESSAGE[] = "@@users-reset";
static const char USER_MESSAGE_PREFIX[] = "@@user ";

static void cleanup(int signo)
{
    (void)signo;
    if (g_server_queue != (mqd_t)-1) {
        mq_close(g_server_queue);
        mq_unlink(CHAT_SERVER_QUEUE);
    }
    _exit(0);
}

static int find_client(ClientInfo clients[], int count, pid_t pid)
{
    for (int i = 0; i < count; ++i) {
        if (clients[i].pid == pid) {
            return i;
        }
    }

    return -1;
}

static void broadcast(const ClientInfo clients[], int count, const char *text)
{
    ChatMessage out = {0};

    out.command = CHAT_CMD_TEXT;
    snprintf(out.name, sizeof(out.name), "server");
    snprintf(out.text, sizeof(out.text), "%s", text);

    for (int i = 0; i < count; ++i) {
        if (mq_send(clients[i].queue, (const char *)&out, sizeof(out), 0) == -1) {
            perror("mq_send");
        }
    }
}

static int open_client_queue(pid_t pid)
{
    char queue_name[CHAT_QUEUE_NAME_SIZE];

    chat_make_client_queue_name(queue_name, sizeof(queue_name), pid);
    return mq_open(queue_name, O_WRONLY);
}

static void send_to_client(mqd_t queue, const char *text)
{
    ChatMessage out = {0};

    out.command = CHAT_CMD_TEXT;
    snprintf(out.name, sizeof(out.name), "server");
    snprintf(out.text, sizeof(out.text), "%s", text);

    if (mq_send(queue, (const char *)&out, sizeof(out), 0) == -1) {
        perror("mq_send");
    }
}

static void send_user_snapshot(mqd_t queue, const ClientInfo clients[], int count)
{
    char line[MAX_NAME + 16];

    send_to_client(queue, USERS_RESET_MESSAGE);
    for (int i = 0; i < count; ++i) {
        snprintf(line, sizeof(line), "%s%s", USER_MESSAGE_PREFIX, clients[i].name);
        send_to_client(queue, line);
    }
}

int main(void)
{
    ClientInfo clients[CHAT_MAX_CLIENTS] = {0};
    int client_count = 0;
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = CHAT_QUEUE_MAX_MESSAGES,
        .mq_msgsize = sizeof(ChatMessage),
        .mq_curmsgs = 0
    };

    if (mq_unlink(CHAT_SERVER_QUEUE) == -1 && errno != ENOENT) {
        perror("mq_unlink");
        return 1;
    }

    g_server_queue = mq_open(CHAT_SERVER_QUEUE, O_RDONLY | O_CREAT, 0666, &attr);
    if (g_server_queue == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    printf("Chat server started. Press Ctrl+C to stop.\n");

    while (1) {
        ChatMessage msg = {0};
        char line[MAX_NAME + MAX_TEXT + 32];
        int index;

        if (mq_receive(g_server_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            perror("mq_receive");
            cleanup(0);
        }

        if (msg.command == CHAT_CMD_JOIN) {
            if (find_client(clients, client_count, msg.pid) == -1 &&
                client_count < CHAT_MAX_CLIENTS) {
                mqd_t client_queue = open_client_queue(msg.pid);

                if (client_queue == (mqd_t)-1) {
                    perror("mq_open client");
                    continue;
                }

                clients[client_count].pid = msg.pid;
                clients[client_count].queue = client_queue;
                snprintf(clients[client_count].name,
                         sizeof(clients[client_count].name), "%s", msg.name);
                ++client_count;
            }

            index = find_client(clients, client_count, msg.pid);
            if (index != -1) {
                send_user_snapshot(clients[index].queue, clients, client_count);
            }
            snprintf(line, sizeof(line), "%s joined the chat", msg.name);
            printf("%s\n", line);
            broadcast(clients, client_count, line);
            continue;
        }

        if (msg.command == CHAT_CMD_LEAVE) {
            index = find_client(clients, client_count, msg.pid);
            if (index != -1) {
                snprintf(line, sizeof(line), "%s left the chat", clients[index].name);
                mq_close(clients[index].queue);
                for (int i = index; i < client_count - 1; ++i) {
                    clients[i] = clients[i + 1];
                }
                --client_count;
                printf("%s\n", line);
                broadcast(clients, client_count, line);
            }
            continue;
        }

        if (msg.command == CHAT_CMD_TEXT) {
            snprintf(line, sizeof(line), "%s: %s", msg.name, msg.text);
            printf("%s\n", line);
            broadcast(clients, client_count, line);
        }
    }
}
