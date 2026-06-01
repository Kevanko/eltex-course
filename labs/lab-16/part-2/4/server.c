#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define SERVER_PORT 7777
#define BACKLOG 16
#define MAX_CLIENTS 32
#define TCP_INDEX 0
#define UDP_INDEX 1

static void make_time_response(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;

    localtime_r(&now, &tm_now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S\n", &tm_now);
}

static int set_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static int bind_socket(int type)
{
    int fd;
    int opt = 1;
    struct sockaddr_in addr;

    fd = socket(AF_INET, type, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (set_nonblock(fd) == -1) {
        close(fd);
        return -1;
    }

    return fd;
}

static int create_tcp_listener(void)
{
    int fd = bind_socket(SOCK_STREAM);

    if (fd == -1) {
        return -1;
    }

    if (listen(fd, BACKLOG) == -1) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

static void remove_fd(struct pollfd fds[], nfds_t *count, nfds_t index)
{
    close(fds[index].fd);
    fds[index] = fds[*count - 1];
    --(*count);
}

static void handle_tcp_listener(struct pollfd fds[], nfds_t *count)
{
    while (*count < MAX_CLIENTS + 2) {
        int client_fd = accept(fds[TCP_INDEX].fd, NULL, NULL);

        if (client_fd == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                perror("accept");
            }
            return;
        }

        if (set_nonblock(client_fd) == -1) {
            close(client_fd);
            continue;
        }

        fds[*count].fd = client_fd;
        fds[*count].events = POLLIN;
        fds[*count].revents = 0;
        ++(*count);
    }
}

static void handle_udp(int udp_fd)
{
    while (1) {
        char request[64];
        char response[64];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t bytes_read;

        bytes_read = recvfrom(udp_fd, request, sizeof(request) - 1, 0,
                              (struct sockaddr *)&client_addr, &client_len);
        if (bytes_read == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom");
            }
            return;
        }

        request[bytes_read] = '\0';
        make_time_response(response, sizeof(response));
        if (sendto(udp_fd, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, client_len) == -1) {
            perror("sendto");
        }
    }
}

static void handle_tcp_client(struct pollfd fds[], nfds_t *count, nfds_t index)
{
    char request[64];
    char response[64];
    ssize_t bytes_read;

    bytes_read = read(fds[index].fd, request, sizeof(request) - 1);
    if (bytes_read > 0) {
        request[bytes_read] = '\0';
        make_time_response(response, sizeof(response));
        if (write(fds[index].fd, response, strlen(response)) == -1) {
            perror("write");
        }
    } else if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("read");
    }

    remove_fd(fds, count, index);
}

int main(void)
{
    struct pollfd fds[MAX_CLIENTS + 2];
    nfds_t count = 2;

    fds[TCP_INDEX].fd = create_tcp_listener();
    if (fds[TCP_INDEX].fd == -1) {
        return 1;
    }
    fds[TCP_INDEX].events = POLLIN;

    fds[UDP_INDEX].fd = bind_socket(SOCK_DGRAM);
    if (fds[UDP_INDEX].fd == -1) {
        close(fds[TCP_INDEX].fd);
        return 1;
    }
    fds[UDP_INDEX].events = POLLIN;

    printf("Multiprotocol server started on tcp/udp port %d\n", SERVER_PORT);

    while (1) {
        int ready = poll(fds, count, -1);

        if (ready == -1) {
            if (errno != EINTR) {
                perror("poll");
            }
            continue;
        }

        if (fds[TCP_INDEX].revents & POLLIN) {
            handle_tcp_listener(fds, &count);
        }

        if (fds[UDP_INDEX].revents & POLLIN) {
            handle_udp(fds[UDP_INDEX].fd);
        }

        for (nfds_t i = 2; i < count; ++i) {
            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                handle_tcp_client(fds, &count, i);
                --i;
            }
        }
    }

    return 0;
}
