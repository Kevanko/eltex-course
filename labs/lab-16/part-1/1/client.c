#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_SOCKET_PATH "/tmp/lab16_1_server.socket"
#define CLIENT_SOCKET_PATH "/tmp/lab16_1_client.socket"

int main(void)
{
    int client_fd;
    struct sockaddr_un server_addr;
    struct sockaddr_un client_addr;
    char buffer[128];
    ssize_t bytes_read;

    unlink(CLIENT_SOCKET_PATH);

    client_fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_LOCAL;
    strncpy(client_addr.sun_path, CLIENT_SOCKET_PATH, sizeof(client_addr.sun_path) - 1);

    if (bind(client_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1) {
        perror("bind");
        close(client_fd);
        unlink(CLIENT_SOCKET_PATH);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (sendto(client_fd, "hello!", 6, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(client_fd);
        unlink(CLIENT_SOCKET_PATH);
        return 1;
    }

    bytes_read = recvfrom(client_fd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (bytes_read == -1) {
        perror("recvfrom");
        close(client_fd);
        unlink(CLIENT_SOCKET_PATH);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    close(client_fd);
    unlink(CLIENT_SOCKET_PATH);

    return 0;
}
