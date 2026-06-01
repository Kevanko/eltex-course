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

#define SERVER_PORT 7777
#define BACKLOG 16
#define WORKER_COUNT 3
#define QUEUE_SIZE 16

typedef struct {
    int items[QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ClientQueue;

static ClientQueue queue = {
    .head = 0,
    .tail = 0,
    .count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .not_empty = PTHREAD_COND_INITIALIZER,
    .not_full = PTHREAD_COND_INITIALIZER
};

static void make_time_response(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;

    localtime_r(&now, &tm_now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S\n", &tm_now);
}

static int create_listener(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(server_fd);
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

static void queue_push(int client_fd)
{
    pthread_mutex_lock(&queue.mutex);

    while (queue.count == QUEUE_SIZE) {
        pthread_cond_wait(&queue.not_full, &queue.mutex);
    }

    queue.items[queue.tail] = client_fd;
    queue.tail = (queue.tail + 1) % QUEUE_SIZE;
    ++queue.count;

    pthread_cond_signal(&queue.not_empty);
    pthread_mutex_unlock(&queue.mutex);
}

static int queue_pop(void)
{
    int client_fd;

    pthread_mutex_lock(&queue.mutex);

    while (queue.count == 0) {
        pthread_cond_wait(&queue.not_empty, &queue.mutex);
    }

    client_fd = queue.items[queue.head];
    queue.head = (queue.head + 1) % QUEUE_SIZE;
    --queue.count;

    pthread_cond_signal(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);

    return client_fd;
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
    int worker_id = *(int *)arg;

    while (1) {
        int client_fd = queue_pop();

        printf("worker %d handles client\n", worker_id);
        serve_time(client_fd);
        close(client_fd);
    }

    return NULL;
}

int main(void)
{
    int server_fd;
    int worker_ids[WORKER_COUNT];

    server_fd = create_listener();
    if (server_fd == -1) {
        return 1;
    }

    for (int i = 0; i < WORKER_COUNT; ++i) {
        pthread_t thread;

        worker_ids[i] = i + 1;
        if (pthread_create(&thread, NULL, worker_main, &worker_ids[i]) != 0) {
            perror("pthread_create");
            close(server_fd);
            return 1;
        }
        pthread_detach(thread);
    }

    printf("Producer/consumer server started on port %d\n", SERVER_PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
            }
            continue;
        }

        queue_push(client_fd);
    }

    close(server_fd);
    return 0;
}
