#ifndef CHAT_H
#define CHAT_H

#include <mqueue.h>
#include <sys/types.h>

#define MAX_NAME 64
#define MAX_TEXT 256
#define CHAT_MAX_CLIENTS 16
#define CHAT_MAX_MESSAGES 64
#define CHAT_QUEUE_NAME_SIZE 64
#define CHAT_SERVER_QUEUE "/chat_server"
#define CHAT_QUEUE_MAX_MESSAGES 10

enum ChatCommand {
    CHAT_CMD_JOIN = 1,
    CHAT_CMD_TEXT = 2,
    CHAT_CMD_LEAVE = 3
};

typedef struct {
    long mtype;
    pid_t pid;
    int command;
    char name[MAX_NAME];
    char text[MAX_TEXT];
} ChatMessage;

static inline void chat_make_client_queue_name(char *buffer, size_t size, pid_t pid)
{
    snprintf(buffer, size, "/chat_client_%ld", (long)pid);
}

#endif
