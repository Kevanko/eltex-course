#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 7777
#define CLIENT_PORT 12701
#define IFACE_NAME "lo"

static int wait_response(int sock_fd)
{
    char packet[512];
    size_t offsets[] = {0, 4, sizeof(struct ethhdr)};

    while (1) {
        ssize_t bytes_read = recv(sock_fd, packet, sizeof(packet), 0);

        if (bytes_read == -1) {
            perror("recv");
            return 1;
        }

        for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
            struct iphdr *ip_header;
            struct udphdr *udp_header;
            char *data;
            size_t ip_offset = offsets[i];

            if ((size_t)bytes_read < ip_offset + sizeof(*ip_header)) {
                continue;
            }

            ip_header = (struct iphdr *)(packet + ip_offset);
            if (ip_header->version != 4 || ip_header->protocol != IPPROTO_UDP) {
                continue;
            }

            udp_header = (struct udphdr *)((char *)ip_header + ip_header->ihl * 4);
            if (ntohs(udp_header->source) != SERVER_PORT ||
                ntohs(udp_header->dest) != CLIENT_PORT) {
                continue;
            }

            data = (char *)(udp_header + 1);
            data[ntohs(udp_header->len) - sizeof(*udp_header)] = '\0';
            printf("Received: %s\n", data);
            return 0;
        }
    }
}

int main(void)
{
    int sock_fd;
    int recv_fd;
    unsigned int ifindex;
    struct sockaddr_ll bind_addr;
    struct sockaddr_in server_addr;
    char packet[256];
    struct udphdr *udp_header = (struct udphdr *)packet;
    const char text[] = "raw udp header";
    size_t packet_len = sizeof(*udp_header) + strlen(text);

    sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock_fd == -1) {
        perror("socket");
        return 1;
    }

    recv_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (recv_fd == -1) {
        perror("socket");
        close(sock_fd);
        return 1;
    }

    ifindex = if_nametoindex(IFACE_NAME);
    if (ifindex == 0) {
        perror("if_nametoindex");
        close(recv_fd);
        close(sock_fd);
        return 1;
    }

    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sll_family = AF_PACKET;
    bind_addr.sll_protocol = htons(ETH_P_ALL);
    bind_addr.sll_ifindex = (int)ifindex;
    if (bind(recv_fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) == -1) {
        perror("bind");
        close(recv_fd);
        close(sock_fd);
        return 1;
    }

    memset(packet, 0, sizeof(packet));
    udp_header->source = htons(CLIENT_PORT);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons((uint16_t)packet_len);
    memcpy(udp_header + 1, text, strlen(text));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1) {
        perror("inet_pton");
        close(recv_fd);
        close(sock_fd);
        return 1;
    }

    if (sendto(sock_fd, packet, packet_len, 0,
        (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("sendto");
        close(recv_fd);
        close(sock_fd);
        return 1;
    }

    if (wait_response(recv_fd) != 0) {
        close(recv_fd);
        close(sock_fd);
        return 1;
    }

    close(recv_fd);
    close(sock_fd);
    return 0;
}
