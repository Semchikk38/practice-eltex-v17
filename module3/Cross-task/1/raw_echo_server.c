#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SIZE 65536

struct client_entry {
    struct in_addr ip;
    uint16_t port;
    int counter;
    int active;
};

static volatile sig_atomic_t running = 1;

void signal_handler(int sig) {
    running = 0;
}

int main() {
    int sock;
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    char buffer[BUFFER_SIZE];
    struct client_entry clients[MAX_CLIENTS];
    memset(clients, 0, sizeof(clients));
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    printf("Echo server listening on raw UDP port %d\n", SERVER_PORT);
    
    while (running) {
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0,
                             (struct sockaddr *)&src_addr, &addr_len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("recvfrom");
            break;
        }
        
        // Проверка минимальной длины: IP-заголовок + UDP-заголовок
        if (n < (ssize_t)sizeof(struct iphdr) + (ssize_t)sizeof(struct udphdr)) {
            continue;
        }
        
        struct iphdr *ip = (struct iphdr *)buffer;
        int ip_header_len = ip->ihl * 4;
        if (ip_header_len < (int)sizeof(struct iphdr) || ip_header_len > n) {
            continue;
        }
        
        struct udphdr *udp = (struct udphdr *)(buffer + ip_header_len);
        // Фильтрация по порту назначения
        if (ntohs(udp->dest) != SERVER_PORT) {
            continue;
        }
        
        char *data = buffer + ip_header_len + sizeof(struct udphdr);
        int data_len = n - ip_header_len - sizeof(struct udphdr);
        if (data_len < 0) data_len = 0;
        
        // Обработка сообщения о закрытии "CLOSE"
        if (data_len == 5 && memcmp(data, "CLOSE", 5) == 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].active &&
                    clients[i].ip.s_addr == ip->saddr &&
                    clients[i].port == udp->source) {
                    clients[i].active = 0;
                    printf("Client %s:%d closed, counter reset\n",
                           inet_ntoa(*(struct in_addr *)&ip->saddr), ntohs(udp->source));
                    break;
                }
            }
            continue;
        }
        
        // Поиск или создание записи клиента
        int idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].active &&
                clients[i].ip.s_addr == ip->saddr &&
                clients[i].port == udp->source) {
                idx = i;
                break;
            }
        }
        if (idx == -1) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i].active) {
                    idx = i;
                    clients[i].active = 1;
                    clients[i].ip.s_addr = ip->saddr;
                    clients[i].port = udp->source;
                    clients[i].counter = 0;
                    break;
                }
            }
        }
        if (idx == -1) {
            fprintf(stderr, "Too many clients\n");
            continue;
        }
        
        clients[idx].counter++;
        int counter = clients[idx].counter;
        
        // Формирование ответа: данные + пробел + номер
        char response_data[BUFFER_SIZE];
        int resp_len = 0;
        if (data_len > 0) {
            memcpy(response_data, data, data_len);
            resp_len = data_len;
        }
        char num_str[32];
        int num_len = snprintf(num_str, sizeof(num_str), "%d", counter);
        if (resp_len + 1 + num_len >= (int)sizeof(response_data)) {
            fprintf(stderr, "Response too large\n");
            continue;
        }
        response_data[resp_len] = ' ';
        memcpy(response_data + resp_len + 1, num_str, num_len);
        resp_len += 1 + num_len;
        
        // Подготовка UDP-заголовка для ответа
        char sendbuf[BUFFER_SIZE];
        struct udphdr *resp_udp = (struct udphdr *)sendbuf;
        resp_udp->source = htons(SERVER_PORT);
        resp_udp->dest = udp->source;      // порт клиента
        resp_udp->len = htons(sizeof(struct udphdr) + resp_len);
        resp_udp->check = 0;               // контрольная сумма не вычисляется
        
        memcpy(sendbuf + sizeof(struct udphdr), response_data, resp_len);
        
        // Адрес назначения — IP и порт клиента
        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = ip->saddr;
        dest_addr.sin_port = udp->source;
        
        ssize_t sent = sendto(sock, sendbuf, sizeof(struct udphdr) + resp_len, 0,
                              (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (sent < 0) {
            perror("sendto");
        } else {
            printf("Echoed to %s:%d counter %d\n",
                   inet_ntoa(*(struct in_addr *)&ip->saddr), ntohs(udp->source), counter);
        }
    }
    
    close(sock);
    printf("Server terminated.\n");
    return 0;
}