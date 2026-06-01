#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main(void)
{
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[128];
    ssize_t bytes_read;

    client_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path, "/tmp/lab16_2.socket", sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        return 1;
    }

    if (write(client_fd, "hello!", 6) == -1) {
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
    printf("Received: %s\n", buffer);

    close(client_fd);

    return 0;
}
