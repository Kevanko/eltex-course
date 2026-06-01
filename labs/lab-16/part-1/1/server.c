#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/lab16_1_server.socket"

int main(void)
{
    int server_fd;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;
    char buffer[128];
    ssize_t bytes_read;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    client_addr_len = sizeof(client_addr);
    bytes_read = recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0,
        (struct sockaddr*)&client_addr, &client_addr_len);
    if (bytes_read == -1) {
        perror("recvfrom");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    if (sendto(server_fd, "hi!", 3, 0, (struct sockaddr*)&client_addr, client_addr_len) == -1) {
        perror("sendto");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
