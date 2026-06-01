#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BROADCAST_IP "255.255.255.255"
#define SERVER_PORT 7777

int main(void)
{
    int client_fd;
    int opt = 1;
    struct sockaddr_in server_addr;
    char buffer[256];
    ssize_t bytes_read;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    if (setsockopt(client_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(client_fd);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, BROADCAST_IP, &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(client_fd);
        return 1;
    }

    if (sendto(client_fd, "broadcast message", 17, 0,
        (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(client_fd);
        return 1;
    }

    bytes_read = recvfrom(client_fd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (bytes_read == -1) {
        perror("recvfrom");
        close(client_fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    close(client_fd);
    return 0;
}

