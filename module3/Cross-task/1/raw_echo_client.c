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
#include <sys/select.h>

#define SERVER_PORT 8888
#define CLIENT_PORT 55555
#define BUFFER_SIZE 65536

static volatile sig_atomic_t should_close = 0;

void signal_handler(int sig) {
    should_close = 1;
}

void send_close_message(int sock, const struct sockaddr_in *server_addr) {
    char buffer[BUFFER_SIZE];
    struct udphdr *udp = (struct udphdr *)buffer;
    udp->source = htons(CLIENT_PORT);
    udp->dest = htons(SERVER_PORT);
    const char *msg = "CLOSE";
    int msg_len = 5;
    udp->len = htons(sizeof(struct udphdr) + msg_len);
    udp->check = 0;
    memcpy(buffer + sizeof(struct udphdr), msg, msg_len);
    sendto(sock, buffer, sizeof(struct udphdr) + msg_len, 0,
           (struct sockaddr *)server_addr, sizeof(*server_addr));
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char input[256];
    char server_ip_str[INET_ADDRSTRLEN];
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (argc > 1) {
        if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
            fprintf(stderr, "Invalid server address\n");
            exit(EXIT_FAILURE);
        }
    } else {
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip_str, sizeof(server_ip_str));
    
    printf("Raw UDP echo client. Type messages, Ctrl+C to exit.\n");
    printf("Server IP: %s\n", server_ip_str);
    
    while (!should_close) {
        printf("> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;
        
        if (should_close) {
            send_close_message(sock, &server_addr);
            break;
        }
        
        // Формирование UDP-пакета
        char buffer[BUFFER_SIZE];
        struct udphdr *udp = (struct udphdr *)buffer;
        udp->source = htons(CLIENT_PORT);
        udp->dest = htons(SERVER_PORT);
        udp->len = htons(sizeof(struct udphdr) + len);
        udp->check = 0;
        memcpy(buffer + sizeof(struct udphdr), input, len);
        
        if (sendto(sock, buffer, sizeof(struct udphdr) + len, 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("sendto");
            continue;
        }
        
        // Ожидание ответа с таймаутом
        // Цикл нужен, чтобы игнорировать свои собственные пакеты
        int response_received = 0;
        int attempts = 0;
        const int max_attempts = 5;  // Максимум 5 попыток (10 секунд)
        
        while (!response_received && attempts < max_attempts && !should_close) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sock, &readfds);
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            
            int ready = select(sock + 1, &readfds, NULL, NULL, &tv);
            if (ready < 0) {
                if (errno == EINTR) {
                    if (should_close) {
                        send_close_message(sock, &server_addr);
                        break;
                    }
                    continue;
                }
                perror("select");
                break;
            } else if (ready == 0) {
                printf("No response from server.\n");
                break;
            }
            
            // Приём пакета
            struct sockaddr_in from_addr;
            socklen_t from_len = sizeof(from_addr);
            ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0,
                                 (struct sockaddr *)&from_addr, &from_len);
            if (n < 0) {
                if (errno == EINTR) {
                    if (should_close) {
                        send_close_message(sock, &server_addr);
                        break;
                    }
                    continue;
                }
                perror("recvfrom");
                break;
            }
            
            // Разбор пакета
            if (n >= (ssize_t)sizeof(struct iphdr) + (ssize_t)sizeof(struct udphdr)) {
                struct iphdr *ip = (struct iphdr *)buffer;
                int ip_header_len = ip->ihl * 4;
                struct udphdr *udp_resp = (struct udphdr *)(buffer + ip_header_len);
                
                // Фильтрация пакетов:
                // 1. Игнорируем свои собственные пакеты (source=CLIENT_PORT, dest=SERVER_PORT)
                // 2. Принимаем только пакеты от сервера (source=SERVER_PORT, dest=CLIENT_PORT)
                
                if (ntohs(udp_resp->source) == SERVER_PORT && 
                    ntohs(udp_resp->dest) == CLIENT_PORT) {
                    // Это ответ от сервера
                    char *data = buffer + ip_header_len + sizeof(struct udphdr);
                    int data_len = n - ip_header_len - sizeof(struct udphdr);
                    
                    if (data_len > 0) {
                        printf("Server: %.*s\n", data_len, data);
                        response_received = 1;  // Получили ответ
                    }
                } else if (ntohs(udp_resp->source) == CLIENT_PORT && 
                           ntohs(udp_resp->dest) == SERVER_PORT) {
                    // Это наш собственный пакет, отражённый loopback
                    // Тихо игнорируем и ждём следующий пакет
                    attempts++;
                    continue;
                } else {
                    // Пакет от кого-то другого - тоже игнорируем
                    attempts++;
                    continue;
                }
            }
        }
    }
    
    if (should_close) {
        send_close_message(sock, &server_addr);
    }
    
    close(sock);
    printf("Client terminated.\n");
    return 0;
}