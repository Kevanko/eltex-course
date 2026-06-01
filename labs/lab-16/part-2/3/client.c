#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777

int main(void)
{
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[128];
    ssize_t bytes_read;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    if (write(client_fd, "time", 4) == -1) {
        perror("write");
        close(client_fd);
        return 1;
    }

    bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(client_fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Current time: %s", buffer);

    close(client_fd);
    return 0;
}

