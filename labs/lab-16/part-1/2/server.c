#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void)
{
    int server_fd;
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[128];
    ssize_t bytes_read;

    unlink("/tmp/lab16_2.socket");

    server_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, "/tmp/lab16_2.socket", sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        unlink("/tmp/lab16_2.socket");
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        unlink("/tmp/lab16_2.socket");
        return 1;
    }

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        unlink("/tmp/lab16_2.socket");
        return 1;
    }

    bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(client_fd);
        close(server_fd);
        unlink("/tmp/lab16_2.socket");
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    if (write(client_fd, "hi!", 3) == -1) {
        perror("write");
        close(client_fd);
        close(server_fd);
        unlink("/tmp/lab16_2.socket");
        return 1;
    }

    close(client_fd);
    close(server_fd);
    unlink("/tmp/lab16_2.socket");

    return 0;
}
