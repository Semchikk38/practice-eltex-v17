#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>

#define MAX_MSG 10
#define MSG_SIZE 256

mqd_t mq_in, mq_out;
int is_creator = 0;
char name1[50], name2[50];

void sigint_handler(int sig) {
    char *bye = "EXIT";
    mq_send(mq_out, bye, strlen(bye) + 1, 99); // Приоритет 99 - сигнал выхода
    mq_close(mq_in); mq_close(mq_out);
    if (is_creator) {
        mq_unlink(name1);
        mq_unlink(name2);
        printf("\nОчереди удалены.\n");
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { printf("Usage: %s <chat_name>\n", argv[0]); return 1; }
    
    snprintf(name1, 50, "/%s_1", argv[1]);
    snprintf(name2, 50, "/%s_2", argv[1]);

    struct mq_attr attr;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_flags = 0;

    mqd_t q1 = mq_open(name1, O_CREAT | O_EXCL | O_RDWR, 0644, &attr);
    if (q1 >= 0) {
        is_creator = 1;
        mq_in = q1;
        mq_out = mq_open(name2, O_CREAT | O_RDWR, 0644, &attr);
        printf("Вы создали чат. Ожидание собеседника...\n");
    } else {
        mq_out = mq_open(name1, O_RDWR);
        mq_in = mq_open(name2, O_RDWR);
        printf("Вы подключились к чату.\n");
    }

    signal(SIGINT, sigint_handler);

    pid_t pid = fork();
    if (pid == 0) { // Процесс чтения
        char buf[MSG_SIZE];
        unsigned int prio;
        while (1) {
            ssize_t n = mq_receive(mq_in, buf, MSG_SIZE, &prio);
            if (n <= 0) break;
            if (prio == 99 || strcmp(buf, "EXIT") == 0) {
                printf("\nСобеседник вышел из чата.\n");
                break;
            }
            printf("\n[Собеседник]: %s\n> ", buf);
            fflush(stdout);
        }
        exit(0);
    } else { // Процесс записи
        char buf[MSG_SIZE];
        printf("> ");
        while (fgets(buf, MSG_SIZE, stdin) != NULL) {
            buf[strcspn(buf, "\n")] = 0;
            if (strlen(buf) == 0) continue;
            mq_send(mq_out, buf, strlen(buf) + 1, 1);
            if (strcmp(buf, "exit") == 0) break;
            printf("> ");
        }
        kill(pid, SIGTERM);
        sigint_handler(SIGINT);
    }
    return 0;
}