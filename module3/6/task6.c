#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#define PORT 8888
#define BROADCAST_IP "255.255.255.255"
int sockfd;

void sigint_handler(int sig) {
    char msg[50];
    snprintf(msg, sizeof(msg), "LEFT:%d", getpid());
    struct sockaddr_in addr = {AF_INET, htons(PORT), inet_addr(BROADCAST_IP)};
    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sockfd);
    printf("\nОтправлено LEFT, выход.\n");
    exit(0);
}

int main() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in my_addr = {AF_INET, htons(PORT), INADDR_ANY};
    bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));

    struct sockaddr_in bcast_addr = {AF_INET, htons(PORT), inet_addr(BROADCAST_IP)};
    char msg[50];
    snprintf(msg, sizeof(msg), "NEW:%d", getpid());
    sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)&bcast_addr, sizeof(bcast_addr));

    signal(SIGINT, sigint_handler);
    printf("Чат запущен. PID: %d\n", getpid());

    char buf[256];
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds)) {
            struct sockaddr_in sender;
            socklen_t len = sizeof(sender);
            ssize_t n = recvfrom(sockfd, buf, sizeof(buf)-1, 0, (struct sockaddr*)&sender, &len);
            if (n > 0) {
                buf[n] = '\0';
                printf("\n[Сеть] %s\n> ", buf);
                fflush(stdout);
            }
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buf, sizeof(buf), stdin);
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) > 0) {
                sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&bcast_addr, sizeof(bcast_addr));
            }
            printf("> ");
        }
    }
    return 0;
}