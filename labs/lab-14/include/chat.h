#ifndef CHAT_H
#define CHAT_H

#include <semaphore.h>
#include <sys/types.h>

#define MAX_NAME 64
#define MAX_TEXT 256
#define CHAT_MAX_CLIENTS 16
#define CHAT_MAX_MESSAGES 64
#define CHAT_MAX_COMMANDS 32
#define CHAT_LINE_LENGTH (MAX_NAME + MAX_TEXT + 8)

#define POSIX_EXCHANGE_SHM_NAME "/lab14_posix_exchange"
#define CHAT_SHM_NAME "/lab14_chat_room"

#define SYSV_SHM_KEY_PATH "include/chat.h"
#define SYSV_EXCHANGE_KEY_ID 14
#define SYSV_SEM_KEY_ID 15

enum ChatCommand {
    CHAT_CMD_JOIN = 1,
    CHAT_CMD_TEXT = 2,
    CHAT_CMD_LEAVE = 3
};

typedef struct {
    sem_t server_ready;
    sem_t client_ready;
    char server_text[128];
    char client_text[128];
} PosixExchangeMemory;

typedef struct {
    char server_text[128];
    char client_text[128];
} SysvExchangeMemory;

typedef struct {
    pid_t pid;
    char name[MAX_NAME];
} ChatUser;

typedef struct {
    int command;
    pid_t pid;
    char name[MAX_NAME];
    char text[MAX_TEXT];
} ChatRequest;

typedef struct {
    sem_t lock;
    unsigned int version;
    int message_count;
    char messages[CHAT_MAX_MESSAGES][CHAT_LINE_LENGTH];
    ChatUser users[CHAT_MAX_CLIENTS];
    int user_count;
    ChatRequest commands[CHAT_MAX_COMMANDS];
    int command_head;
    int command_tail;
    int command_count;
} ChatRoom;

#endif
