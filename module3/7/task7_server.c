#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9999
#define MAX_EVENTS 20
#define BUF_SIZE 1048576

int clients[MAX_EVENTS];
int client_count = 0;

void broadcast(int sender_fd, char *buf, ssize_t n) {
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_fd) {
            send(clients[i], buf, n, 0);
        }
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {AF_INET, htons(PORT), INADDR_ANY};
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

    printf("Сервер запущен на порту %d\n", PORT);

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                clients[client_count++] = client_fd;
            } else {
                char *buf = malloc(BUF_SIZE);
                ssize_t count = read(events[i].data.fd, buf, BUF_SIZE);
                if (count <= 0) {
                    close(events[i].data.fd);
                    for(int j=0; j<client_count; j++) {
                        if(clients[j] == events[i].data.fd) {
                            clients[j] = clients[--client_count];
                            break;
                        }
                    }
                } else {
                    broadcast(events[i].data.fd, buf, count);
                }
                free(buf);
            }
        }
    }
    return 0;
}