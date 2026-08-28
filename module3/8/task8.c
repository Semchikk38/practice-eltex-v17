#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <arpa/inet.h>
#include <time.h>

void print_mac(unsigned char *mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: sudo %s <filter: 1=chat(8888), 2=dns(53)>\n", argv[0]);
        return 1;
    }
    int filter = atoi(argv[1]);
    int target_port = (filter == 1) ? 8888 : 53;

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) { perror("Socket (нужен sudo)"); return 1; }

    unsigned char buffer[65536];
    struct sockaddr_ll sll;
    socklen_t sll_len = sizeof(sll);
    time_t start_time = time(NULL);

    printf("Захват начат. Фильтр: %s (порт %d). Нажмите Ctrl+C для остановки.\n", 
           filter == 1 ? "Chat" : "DNS", target_port);

    while (1) {
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&sll, &sll_len);
        if (n < sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)) continue;

        struct ethhdr *eth = (struct ethhdr *)buffer;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;

        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol != IPPROTO_UDP) continue;

        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip->ihl * 4));
        int src_port = ntohs(udp->source);
        int dst_port = ntohs(udp->dest);

        if (src_port != target_port && dst_port != target_port) continue;

        time_t now = time(NULL);
        printf("\n[%ld сек] MAC: ", now - start_time);
        print_mac(eth->h_source); printf(" -> "); print_mac(eth->h_dest);
        
        char src_ip[20], dst_ip[20];
        inet_ntop(AF_INET, &(ip->saddr), src_ip, 20);
        inet_ntop(AF_INET, &(ip->daddr), dst_ip, 20);
        printf(" | IP: %s:%d -> %s:%d\n", src_ip, src_port, dst_ip, dst_port);
    }
    close(sock);
    return 0;
}