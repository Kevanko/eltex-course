#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777
#define DEFAULT_CLIENT_PORT 7776
#define MAX_TEXT 256

static volatile sig_atomic_t g_need_stop = 0;

static void stop_handler(int signo)
{
    (void)signo;
    g_need_stop = 1;
}

static void setup_signals(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = stop_handler;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

static int is_number(const char *text)
{
    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] < '0' || text[i] > '9') {
            return 0;
        }
    }

    return text[0] != '\0';
}

static int send_message(int sock_fd, int client_port, const char *text)
{
    char packet[sizeof(struct udphdr) + MAX_TEXT];
    struct udphdr *udp_header = (struct udphdr *)packet;
    struct sockaddr_in addr;
    size_t text_len = strlen(text);

    memset(packet, 0, sizeof(packet));
    udp_header->source = htons((uint16_t)client_port);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons((uint16_t)(sizeof(*udp_header) + text_len));
    memcpy(udp_header + 1, text, text_len);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) != 1) {
        perror("inet_pton");
        return 1;
    }

    if (sendto(sock_fd, packet, sizeof(*udp_header) + text_len, 0,
        (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("sendto");
        return 1;
    }

    return 0;
}

static int wait_answer(int sock_fd, int client_port)
{
    char packet[512];

    while (1) {
        struct iphdr *ip_header;
        struct udphdr *udp_header;
        char text[MAX_TEXT];
        size_t text_len;
        ssize_t bytes_read = recv(sock_fd, packet, sizeof(packet), 0);

        if (bytes_read == -1) {
            if (errno == EINTR && g_need_stop) {
                return 1;
            }
            perror("recv");
            return 1;
        }

        ip_header = (struct iphdr *)packet;
        if (ip_header->protocol != IPPROTO_UDP) {
            continue;
        }

        udp_header = (struct udphdr *)(packet + ip_header->ihl * 4);
        if (ntohs(udp_header->source) != SERVER_PORT ||
            ntohs(udp_header->dest) != client_port) {
            continue;
        }

        text_len = ntohs(udp_header->len) - sizeof(*udp_header);
        if (text_len >= sizeof(text)) {
            text_len = sizeof(text) - 1;
        }

        memcpy(text, udp_header + 1, text_len);
        text[text_len] = '\0';
        printf("Server: %s\n", text);
        return 0;
    }
}

int main(int argc, char *argv[])
{
    int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    int client_port = DEFAULT_CLIENT_PORT;
    int first_message = 1;
    char line[MAX_TEXT];

    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    setup_signals();

    if (argc > 1 && is_number(argv[1])) {
        client_port = atoi(argv[1]);
        first_message = 2;
    }

    if (argc == first_message) {
        while (!g_need_stop) {
            printf("Enter message: ");
            fflush(stdout);

            if (fgets(line, sizeof(line), stdin) == NULL) {
                break;
            }

            line[strcspn(line, "\n")] = '\0';
            if (line[0] == '\0') {
                continue;
            }

            if (strcmp(line, "exit") == 0 || strcmp(line, "close") == 0) {
                break;
            }

            if (send_message(sock_fd, client_port, line) != 0 ||
                wait_answer(sock_fd, client_port) != 0) {
                send_message(sock_fd, client_port, "close");
                close(sock_fd);
                return 1;
            }
        }
    } else {
        for (int i = first_message; i < argc && !g_need_stop; ++i) {
            printf("Client: %s\n", argv[i]);
            if (send_message(sock_fd, client_port, argv[i]) != 0 ||
                wait_answer(sock_fd, client_port) != 0) {
                send_message(sock_fd, client_port, "close");
                close(sock_fd);
                return 1;
            }
        }
    }

    send_message(sock_fd, client_port, "close");
    close(sock_fd);
    return 0;
}
