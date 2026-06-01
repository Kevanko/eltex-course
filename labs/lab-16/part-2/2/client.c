#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define LISTENER_PORT 7777

static int connect_to_port(int port)
{
    int client_fd;
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(client_fd);
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int main(void)
{
    int listener_fd;
    int worker_fd;
    int worker_port;
    char buffer[128];
    ssize_t bytes_read;

    listener_fd = connect_to_port(LISTENER_PORT);
    if (listener_fd == -1) {
        return 1;
    }

    if (write(listener_fd, "time", 4) == -1) {
        perror("write");
        close(listener_fd);
        return 1;
    }

    bytes_read = read(listener_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(listener_fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    close(listener_fd);

    if (sscanf(buffer, "PORT %d", &worker_port) != 1) {
        printf("%s", buffer);
        return 1;
    }

    worker_fd = connect_to_port(worker_port);
    if (worker_fd == -1) {
        return 1;
    }

    if (write(worker_fd, "time", 4) == -1) {
        perror("write");
        close(worker_fd);
        return 1;
    }

    bytes_read = read(worker_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(worker_fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Current time: %s", buffer);

    close(worker_fd);
    return 0;
}

