#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ncurses.h>

#include "chat.h"
#include "chat_ui.h"

typedef struct {
    int shm_fd;
    ChatRoom *room;
    pid_t pid;
    pthread_t receiver;
    bool receiver_started;
    bool running;
    unsigned int version;
    char name[MAX_NAME];
    char input[MAX_TEXT];
    int input_len;
    char messages[CHAT_MAX_MESSAGES][CHAT_UI_LINE_LENGTH];
    int message_count;
    char users[CHAT_MAX_CLIENTS][MAX_NAME];
    int user_count;
    pthread_mutex_t state_lock;
} ClientState;

static void sleep_briefly(void)
{
    const struct timespec delay = {
        .tv_sec = 0,
        .tv_nsec = 100000000
    };

    nanosleep(&delay, NULL);
}

static void client_state_init(ClientState *state)
{
    memset(state, 0, sizeof(*state));
    state->shm_fd = -1;
    state->running = true;
    pthread_mutex_init(&state->state_lock, NULL);
}

static void client_state_destroy(ClientState *state)
{
    if (state->room != NULL) {
        munmap(state->room, sizeof(*state->room));
    }
    if (state->shm_fd != -1) {
        close(state->shm_fd);
    }

    pthread_mutex_destroy(&state->state_lock);
}

static void client_push_message(ClientState *state, const char *text)
{
    pthread_mutex_lock(&state->state_lock);

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

    pthread_mutex_unlock(&state->state_lock);
}

static void client_copy_room_state_locked(ClientState *state)
{
    state->message_count = state->room->message_count;
    for (int i = 0; i < state->room->message_count; ++i) {
        snprintf(state->messages[i], sizeof(state->messages[i]), "%s",
                 state->room->messages[i]);
    }

    state->user_count = state->room->user_count;
    for (int i = 0; i < state->room->user_count; ++i) {
        snprintf(state->users[i], sizeof(state->users[i]), "%s", state->room->users[i].name);
    }
}

static void client_refresh_from_room(ClientState *state)
{
    if (sem_wait(&state->room->lock) == -1) {
        return;
    }

    if (state->version != state->room->version) {
        pthread_mutex_lock(&state->state_lock);
        client_copy_room_state_locked(state);
        state->version = state->room->version;
        pthread_mutex_unlock(&state->state_lock);
    }

    sem_post(&state->room->lock);
}

static void client_snapshot_state(ClientState *state,
                                  char messages[][CHAT_UI_LINE_LENGTH],
                                  int *message_count,
                                  char users[][MAX_NAME],
                                  int *user_count)
{
    pthread_mutex_lock(&state->state_lock);

    *message_count = state->message_count;
    for (int i = 0; i < state->message_count; ++i) {
        snprintf(messages[i], sizeof(messages[i]), "%s", state->messages[i]);
    }

    *user_count = state->user_count;
    for (int i = 0; i < state->user_count; ++i) {
        snprintf(users[i], sizeof(users[i]), "%s", state->users[i]);
    }

    pthread_mutex_unlock(&state->state_lock);
}

static void *receiver_thread(void *arg)
{
    ClientState *state = arg;

    while (state->running) {
        client_refresh_from_room(state);
        sleep_briefly();
    }

    return NULL;
}

static int client_connect(ClientState *state)
{
    state->shm_fd = shm_open(CHAT_SHM_NAME, O_RDWR, 0666);
    if (state->shm_fd == -1) {
        perror("shm_open");
        return -1;
    }

    state->room = mmap(NULL, sizeof(*state->room),
                       PROT_READ | PROT_WRITE, MAP_SHARED, state->shm_fd, 0);
    if (state->room == MAP_FAILED) {
        perror("mmap");
        state->room = NULL;
        return -1;
    }

    state->pid = getpid();
    return 0;
}

static int client_send_command(ClientState *state, int command, const char *text)
{
    int result = -1;

    if (sem_wait(&state->room->lock) == -1) {
        return -1;
    }

    if (state->room->command_count < CHAT_MAX_COMMANDS) {
        ChatRequest *request = &state->room->commands[state->room->command_tail];

        memset(request, 0, sizeof(*request));
        request->command = command;
        request->pid = state->pid;
        snprintf(request->name, sizeof(request->name), "%s", state->name);
        if (text != NULL) {
            snprintf(request->text, sizeof(request->text), "%s", text);
        }

        state->room->command_tail = (state->room->command_tail + 1) % CHAT_MAX_COMMANDS;
        ++state->room->command_count;
        result = 0;
    }

    sem_post(&state->room->lock);

    return result;
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

    state->running = false;
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
        char messages[CHAT_MAX_MESSAGES][CHAT_UI_LINE_LENGTH] = {{0}};
        char users[CHAT_MAX_CLIENTS][MAX_NAME] = {{0}};
        int message_count = 0;
        int user_count = 0;
        int ch;

        client_snapshot_state(state, messages, &message_count, users, &user_count);
        chat_ui_draw(state->name, state->input, messages, message_count, users, user_count);
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

    if (client_connect(&state) == -1) {
        goto cleanup;
    }

    if (client_send_command(&state, CHAT_CMD_JOIN, NULL) == -1) {
        client_push_message(&state, "Failed to join chat");
        goto cleanup;
    }
    joined = true;

    if (client_start_receiver(&state) == -1) {
        goto cleanup;
    }

    client_refresh_from_room(&state);
    client_run_event_loop(&state);
    result = 0;

cleanup:
    if (joined) {
        (void)client_send_command(&state, CHAT_CMD_LEAVE, NULL);
    }

    client_stop_receiver(&state);

    if (ui_ready) {
        chat_ui_shutdown();
    }

    client_state_destroy(&state);

    return result;
}
