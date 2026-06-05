#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 7777
#define MAX_CLIENTS 32
#define MAX_TEXT 256

typedef struct {
    uint32_t ip;
    uint16_t port;
    int count;
} ClientInfo;

static int find_client(ClientInfo clients[], int client_count, uint32_t ip, uint16_t port)
{
    for (int i = 0; i < client_count; ++i) {
        if (clients[i].ip == ip && clients[i].port == port) {
            return i;
        }
    }

    return -1;
}

static int get_client(ClientInfo clients[], int *client_count, uint32_t ip, uint16_t port)
{
    int index = find_client(clients, *client_count, ip, port);

    if (index != -1) {
        return index;
    }

    if (*client_count == MAX_CLIENTS) {
        return -1;
    }

    index = *client_count;
    clients[index].ip = ip;
    clients[index].port = port;
    clients[index].count = 0;
    ++(*client_count);

    return index;
}

static void remove_client(ClientInfo clients[], int *client_count, int index)
{
    clients[index] = clients[*client_count - 1];
    --(*client_count);
}

static int send_answer(int sock_fd, uint32_t dst_ip, uint16_t dst_port, const char *text)
{
    char packet[sizeof(struct udphdr) + MAX_TEXT + 32];
    struct udphdr *udp_header = (struct udphdr *)packet;
    struct sockaddr_in addr;
    size_t text_len = strlen(text);

    memset(packet, 0, sizeof(packet));
    udp_header->source = htons(SERVER_PORT);
    udp_header->dest = htons(dst_port);
    udp_header->len = htons((uint16_t)(sizeof(*udp_header) + text_len));
    memcpy(udp_header + 1, text, text_len);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(dst_port);
    addr.sin_addr.s_addr = dst_ip;

    if (sendto(sock_fd, packet, sizeof(*udp_header) + text_len, 0,
        (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("sendto");
        return 1;
    }

    return 0;
}

int main(void)
{
    int sock_fd;
    ClientInfo clients[MAX_CLIENTS] = {0};
    int client_count = 0;

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    printf("Raw echo server started on port %d\n", SERVER_PORT);

    while (1) {
        char packet[512];
        struct iphdr *ip_header;
        struct udphdr *udp_header;
        char text[MAX_TEXT];
        char answer[MAX_TEXT + 32];
        char ip_text[INET_ADDRSTRLEN];
        ssize_t bytes_read = recv(sock_fd, packet, sizeof(packet), 0);
        size_t text_len;
        int index;

        if (bytes_read == -1) {
            perror("recv");
            continue;
        }

        ip_header = (struct iphdr *)packet;
        if (ip_header->protocol != IPPROTO_UDP) {
            continue;
        }

        udp_header = (struct udphdr *)(packet + ip_header->ihl * 4);
        if (ntohs(udp_header->dest) != SERVER_PORT) {
            continue;
        }

        text_len = ntohs(udp_header->len) - sizeof(*udp_header);
        if (text_len >= sizeof(text)) {
            text_len = sizeof(text) - 1;
        }

        memcpy(text, udp_header + 1, text_len);
        text[text_len] = '\0';

        index = find_client(clients, client_count, ip_header->saddr, ntohs(udp_header->source));
        if (strcmp(text, "close") == 0) {
            if (index != -1) {
                remove_client(clients, &client_count, index);
            }
            inet_ntop(AF_INET, &ip_header->saddr, ip_text, sizeof(ip_text));
            printf("Client %s:%u closed\n", ip_text, ntohs(udp_header->source));
            continue;
        }

        index = get_client(clients, &client_count, ip_header->saddr, ntohs(udp_header->source));
        if (index == -1) {
            continue;
        }

        ++clients[index].count;
        snprintf(answer, sizeof(answer), "%s %d", text, clients[index].count);

        inet_ntop(AF_INET, &ip_header->saddr, ip_text, sizeof(ip_text));
        printf("%s:%u -> %s\n", ip_text, ntohs(udp_header->source), answer);

        send_answer(sock_fd, ip_header->saddr, ntohs(udp_header->source), answer);
    }

    close(sock_fd);
    return 0;
}
