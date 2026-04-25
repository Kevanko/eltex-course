#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "chat.h"
#include <ncurses.h>

#include "chat_ui.h"

static const char USERS_RESET_MESSAGE[] = "@@users-reset";
static const char USER_MESSAGE_PREFIX[] = "@@user ";
static const char JOIN_MESSAGE_SUFFIX[] = " joined the chat";
static const char LEAVE_MESSAGE_SUFFIX[] = " left the chat";

typedef struct {
    mqd_t server_queue;
    mqd_t client_queue;
    pid_t pid;
    pthread_t receiver;
    bool receiver_started;
    char name[MAX_NAME];
    char client_queue_name[CHAT_QUEUE_NAME_SIZE];
    char input[MAX_TEXT];
    int input_len;
    char messages[CHAT_MAX_MESSAGES][CHAT_UI_LINE_LENGTH];
    int message_count;
    char users[CHAT_MAX_CLIENTS][MAX_NAME];
    int user_count;
    pthread_mutex_t message_lock;
} ClientState;

static void client_state_init(ClientState *state)
{
    memset(state, 0, sizeof(*state));
    state->server_queue = (mqd_t)-1;
    state->client_queue = (mqd_t)-1;
    pthread_mutex_init(&state->message_lock, NULL);
}

static void client_state_destroy(ClientState *state)
{
    if (state->server_queue != (mqd_t)-1) {
        mq_close(state->server_queue);
    }
    if (state->client_queue != (mqd_t)-1) {
        mq_close(state->client_queue);
    }
    if (state->client_queue_name[0] != '\0') {
        mq_unlink(state->client_queue_name);
    }

    pthread_mutex_destroy(&state->message_lock);
}

static void client_push_message(ClientState *state, const char *text)
{
    pthread_mutex_lock(&state->message_lock);

    if (state->message_count < CHAT_MAX_MESSAGES) {
        snprintf(state->messages[state->message_count],
                 sizeof(state->messages[state->message_count]), "%s", text);
        ++state->message_count;
    } else {
        memmove(state->messages, state->messages + 1,
                sizeof(state->messages[0]) * (CHAT_MAX_MESSAGES - 1));
        snprintf(state->messages[CHAT_MAX_MESSAGES - 1],
                 sizeof(state->messages[CHAT_MAX_MESSAGES - 1]), "%s", text);
    }

    pthread_mutex_unlock(&state->message_lock);
}

static int client_find_user(const ClientState *state, const char *name)
{
    for (int i = 0; i < state->user_count; ++i) {
        if (strcmp(state->users[i], name) == 0) {
            return i;
        }
    }

    return -1;
}

// Вызывается только при уже взятом message_lock.
static void client_add_user_locked(ClientState *state, const char *name)
{
    if (name[0] == '\0' || client_find_user(state, name) != -1) {
        return;
    }

    if (state->user_count < CHAT_MAX_CLIENTS) {
        snprintf(state->users[state->user_count], sizeof(state->users[state->user_count]),
                 "%s", name);
        ++state->user_count;
    }
}

// Вызывается только при уже взятом message_lock.
static void client_remove_user_locked(ClientState *state, const char *name)
{
    int index = client_find_user(state, name);

    if (index == -1) {
        return;
    }

    if (index < state->user_count - 1) {
        memmove(&state->users[index], &state->users[index + 1],
                sizeof(state->users[0]) * (size_t)(state->user_count - index - 1));
    }

    --state->user_count;
    state->users[state->user_count][0] = '\0';
}

// Достает имя из строк вида "<name> joined/left the chat".
static bool client_extract_name_with_suffix(const char *text,
                                            const char *suffix,
                                            char *name,
                                            size_t size)
{
    size_t text_len = strlen(text);
    size_t suffix_len = strlen(suffix);

    if (text_len <= suffix_len) {
        return false;
    }

    if (strcmp(text + text_len - suffix_len, suffix) != 0) {
        return false;
    }

    if (text_len - suffix_len >= size) {
        return false;
    }

    memcpy(name, text, text_len - suffix_len);
    name[text_len - suffix_len] = '\0';
    return true;
}

// Достает имя отправителя из строки вида "<name>: <text>".
static bool client_extract_sender_name(const char *text, char *name, size_t size)
{
    const char *separator = strstr(text, ": ");
    size_t name_len;

    if (separator == NULL) {
        return false;
    }

    name_len = (size_t)(separator - text);
    if (name_len == 0 || name_len >= size) {
        return false;
    }

    memcpy(name, text, name_len);
    name[name_len] = '\0';
    return true;
}

// Обрабатывает служебные сообщения со снимком списка пользователей.
static void client_process_control_message(ClientState *state, const char *text)
{
    pthread_mutex_lock(&state->message_lock);

    if (strcmp(text, USERS_RESET_MESSAGE) == 0) {
        state->user_count = 0;
        pthread_mutex_unlock(&state->message_lock);
        return;
    }

    if (strncmp(text, USER_MESSAGE_PREFIX, strlen(USER_MESSAGE_PREFIX)) == 0) {
        client_add_user_locked(state, text + strlen(USER_MESSAGE_PREFIX));
        pthread_mutex_unlock(&state->message_lock);
        return;
    }

    pthread_mutex_unlock(&state->message_lock);
}

static bool client_handle_control_message(ClientState *state, const char *text)
{
    if (strcmp(text, USERS_RESET_MESSAGE) != 0 &&
        strncmp(text, USER_MESSAGE_PREFIX, strlen(USER_MESSAGE_PREFIX)) != 0) {
        return false;
    }

    client_process_control_message(state, text);
    return true;
}

static void client_process_received_text(ClientState *state, const char *text)
{
    char name[MAX_NAME] = "";

    if (client_handle_control_message(state, text)) {
        return;
    }

    pthread_mutex_lock(&state->message_lock);

    if (client_extract_name_with_suffix(text, JOIN_MESSAGE_SUFFIX, name, sizeof(name))) {
        client_add_user_locked(state, name);
    } else if (client_extract_name_with_suffix(text, LEAVE_MESSAGE_SUFFIX, name,
                                               sizeof(name))) {
        client_remove_user_locked(state, name);
    } else if (client_extract_sender_name(text, name, sizeof(name))) {
        client_add_user_locked(state, name);
    }

    if (state->message_count < CHAT_MAX_MESSAGES) {
        snprintf(state->messages[state->message_count],
                 sizeof(state->messages[state->message_count]), "%s", text);
        ++state->message_count;
    } else {
        memmove(state->messages, state->messages + 1,
                sizeof(state->messages[0]) * (CHAT_MAX_MESSAGES - 1));
        snprintf(state->messages[CHAT_MAX_MESSAGES - 1],
                 sizeof(state->messages[CHAT_MAX_MESSAGES - 1]), "%s", text);
    }

    pthread_mutex_unlock(&state->message_lock);
}

static void client_snapshot_messages(ClientState *state,
                                     char snapshot[][CHAT_UI_LINE_LENGTH],
                                     int *message_count,
                                     char users_snapshot[][MAX_NAME],
                                     int *user_count)
{
    pthread_mutex_lock(&state->message_lock);

    *message_count = state->message_count;
    for (int i = 0; i < state->message_count; ++i) {
        snprintf(snapshot[i], sizeof(snapshot[i]), "%s", state->messages[i]);
    }

    *user_count = state->user_count;
    for (int i = 0; i < state->user_count; ++i) {
        snprintf(users_snapshot[i], sizeof(users_snapshot[i]), "%s", state->users[i]);
    }

    pthread_mutex_unlock(&state->message_lock);
}

static void *receiver_thread(void *arg)
{
    ClientState *state = arg;

    while (1) {
        ChatMessage msg = {0};

        if (mq_receive(state->client_queue, (char *)&msg, sizeof(msg), NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        client_process_received_text(state, msg.text);
    }

    return NULL;
}

static int client_connect(ClientState *state)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = CHAT_QUEUE_MAX_MESSAGES,
        .mq_msgsize = sizeof(ChatMessage),
        .mq_curmsgs = 0
    };

    state->pid = getpid();
    chat_make_client_queue_name(state->client_queue_name,
                                sizeof(state->client_queue_name), state->pid);

    if (mq_unlink(state->client_queue_name) == -1 && errno != ENOENT) {
        perror("mq_unlink client");
        return -1;
    }

    state->client_queue = mq_open(state->client_queue_name,
                                  O_RDONLY | O_CREAT, 0666, &attr);
    if (state->client_queue == (mqd_t)-1) {
        perror("mq_open client");
        return -1;
    }

    state->server_queue = mq_open(CHAT_SERVER_QUEUE, O_WRONLY);
    if (state->server_queue == (mqd_t)-1) {
        perror("mq_open server");
        return -1;
    }

    return 0;
}

static int client_send_command(const ClientState *state, int command, const char *text)
{
    ChatMessage msg = {0};

    msg.mtype = 1;
    msg.pid = state->pid;
    msg.command = command;
    snprintf(msg.name, sizeof(msg.name), "%s", state->name);

    if (text != NULL) {
        snprintf(msg.text, sizeof(msg.text), "%s", text);
    }

    return mq_send(state->server_queue, (const char *)&msg, sizeof(msg), 0);
}

static int client_start_receiver(ClientState *state)
{
    if (pthread_create(&state->receiver, NULL, receiver_thread, state) != 0) {
        perror("pthread_create");
        return -1;
    }

    state->receiver_started = true;
    return 0;
}

static void client_stop_receiver(ClientState *state)
{
    if (!state->receiver_started) {
        return;
    }

    pthread_cancel(state->receiver);
    pthread_join(state->receiver, NULL);
    state->receiver_started = false;
}

static void client_clear_input(ClientState *state)
{
    state->input[0] = '\0';
    state->input_len = 0;
}

static void client_remove_last_char(ClientState *state)
{
    if (state->input_len == 0) {
        return;
    }

    --state->input_len;
    state->input[state->input_len] = '\0';
}

static void client_append_char(ClientState *state, int ch)
{
    if (ch < 32 || ch >= 127 || state->input_len >= MAX_TEXT - 1) {
        return;
    }

    state->input[state->input_len++] = (char)ch;
    state->input[state->input_len] = '\0';
}

static void client_send_text_message(ClientState *state)
{
    if (state->input_len == 0) {
        return;
    }

    if (client_send_command(state, CHAT_CMD_TEXT, state->input) == -1) {
        client_push_message(state, "Failed to send message");
    }

    client_clear_input(state);
}

static void client_run_event_loop(ClientState *state)
{
    while (1) {
        char snapshot[CHAT_MAX_MESSAGES][CHAT_UI_LINE_LENGTH] = {{0}};
        char users_snapshot[CHAT_MAX_CLIENTS][MAX_NAME] = {{0}};
        int message_count = 0;
        int user_count = 0;
        int ch;

        client_snapshot_messages(state, snapshot, &message_count, users_snapshot, &user_count);
        chat_ui_draw(state->name, state->input, snapshot, message_count,
                     users_snapshot, user_count);
        ch = getch();

        if (ch == ERR) {
            continue;
        }

        if (ch == '\n' || ch == KEY_ENTER) {
            if (strcmp(state->input, "/exit") == 0) {
                break;
            }

            client_send_text_message(state);
            continue;
        }

        if (ch == KEY_F(10)) {
            break;
        }

        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
            client_remove_last_char(state);
            continue;
        }

        client_append_char(state, ch);
    }
}

int main(void)
{
    ClientState state;
    bool joined = false;
    bool ui_ready = false;
    int result = 1;

    client_state_init(&state);

    if (chat_ui_init() == -1) {
        goto cleanup;
    }
    ui_ready = true;

    if (chat_ui_prompt_name(state.name, sizeof(state.name), getpid()) == -1) {
        fprintf(stderr, "Failed to read name\n");
        goto cleanup;
    }

    pthread_mutex_lock(&state.message_lock);
    client_add_user_locked(&state, state.name);
    pthread_mutex_unlock(&state.message_lock);

    if (client_connect(&state) == -1) {
        goto cleanup;
    }

    if (client_send_command(&state, CHAT_CMD_JOIN, NULL) == -1) {
        perror("mq_send");
        goto cleanup;
    }
    joined = true;

    if (client_start_receiver(&state) == -1) {
        goto cleanup;
    }

    client_push_message(&state, "Connected to chat");
    client_run_event_loop(&state);
    result = 0;

cleanup:
    if (ui_ready) {
        chat_ui_shutdown();
    }

    if (joined) {
        (void)client_send_command(&state, CHAT_CMD_LEAVE, NULL);
    }

    client_stop_receiver(&state);
    client_state_destroy(&state);

    return result;
}
