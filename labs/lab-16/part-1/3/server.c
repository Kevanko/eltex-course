#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 7777

int main(void)
{
    int server_fd;
    int opt = 1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    char buffer[128];
    ssize_t bytes_read;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(server_fd);
        return 1;
    }

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    client_addr_len = sizeof(client_addr);
    bytes_read = recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0,
        (struct sockaddr*)&client_addr, &client_addr_len);
    if (bytes_read == -1) {
        perror("recvfrom");
        close(server_fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    if (sendto(server_fd, "hi!", 3, 0, (struct sockaddr*)&client_addr, client_addr_len) == -1) {
        perror("sendto");
        close(server_fd);
        return 1;
    }

    close(server_fd);

    return 0;
}
