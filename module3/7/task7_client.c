#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define PORT 9999
#define BUF_SIZE 1048576

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(PORT)};
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connect failed"); return 1;
    }

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds)) {
            char *buf = malloc(BUF_SIZE);
            ssize_t n = read(sockfd, buf, BUF_SIZE);
            if (n <= 0) { printf("Сервер отключился.\n"); break; }
            
            if (strncmp(buf, "FILE:", 5) == 0) {
                char fname[256]; long fsize;
                sscanf(buf, "FILE:%[^:]:%ld", fname, &fsize);
                char *data_start = strchr(buf, '\n') + 1;
                int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                write(fd, data_start, fsize);
                close(fd);
                printf("\n[Система] Получен и сохранен файл: %s\n> ", fname);
                fflush(stdout);
            } else {
                buf[n] = '\0';
                printf("\n[Чат] %s\n> ", buf);
                fflush(stdout);
            }
            free(buf);
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char cmd[300];
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strcspn(cmd, "\n")] = 0;

            if (strncmp(cmd, "sendfile ", 9) == 0) {
                char *fpath = cmd + 9;
                int fd = open(fpath, O_RDONLY);
                if (fd < 0) { printf("Файл не найден.\n> "); continue; }
                
                struct stat st; fstat(fd, &st);
                char *fdata = malloc(st.st_size);
                read(fd, fdata, st.st_size);
                close(fd);

                char header[300];
                int hlen = snprintf(header, sizeof(header), "FILE:%s:%ld\n", fpath, (long)st.st_size);
                
                send(sockfd, header, hlen, 0);
                send(sockfd, fdata, st.st_size, 0);
                free(fdata);
                printf("Файл отправлен.\n> ");
            } else {
                send(sockfd, cmd, strlen(cmd), 0);
                printf("> ");
            }
        }
    }
    close(sockfd);
    return 0;
}