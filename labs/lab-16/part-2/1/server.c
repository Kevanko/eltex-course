#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 7777
#define BACKLOG 16

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

static void *serve_client(void *arg)
{
    int client_fd = *(int *)arg;
    char request[64];
    char response[64];
    ssize_t bytes_read;

    free(arg);

    bytes_read = read(client_fd, request, sizeof(request) - 1);
    if (bytes_read > 0) {
        request[bytes_read] = '\0';
        make_time_response(response, sizeof(response));
        if (write(client_fd, response, strlen(response)) == -1) {
            perror("write");
        }
    } else if (bytes_read == -1) {
        perror("read");
    }

    close(client_fd);
    return NULL;
}

int main(void)
{
    int server_fd = create_listener();

    if (server_fd == -1) {
        return 1;
    }

    printf("Thread-per-client server started on port %d\n", SERVER_PORT);

    while (1) {
        pthread_t thread;
        int *client_fd = malloc(sizeof(*client_fd));

        if (client_fd == NULL) {
            perror("malloc");
            continue;
        }

        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd == -1) {
            if (errno != EINTR) {
                perror("accept");
            }
            free(client_fd);
            continue;
        }

        if (pthread_create(&thread, NULL, serve_client, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }

        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
