#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777

static int fill_server_addr(struct sockaddr_in *server_addr)
{
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr->sin_addr) != 1) {
        perror("inet_pton");
        return -1;
    }

    return 0;
}

static int run_tcp_client(void)
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

    if (fill_server_addr(&server_addr) == -1) {
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
    printf("TCP time: %s", buffer);

    close(client_fd);
    return 0;
}

static int run_udp_client(void)
{
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[128];
    ssize_t bytes_read;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    if (fill_server_addr(&server_addr) == -1) {
        close(client_fd);
        return 1;
    }

    if (sendto(client_fd, "time", 4, 0,
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
    printf("UDP time: %s", buffer);

    close(client_fd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "udp") == 0) {
        return run_udp_client();
    }

    return run_tcp_client();
}

