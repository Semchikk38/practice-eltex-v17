#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>

#define MSG_KEY 12345
#define MAX_MSG 512
#define MAX_SUBS 50
#define MAX_TOPICS 10

struct msgbuf { long mtype; char mtext[MAX_MSG]; };

struct Subscriber {
    int pid;
    char topics[MAX_TOPICS][50];
    int topic_count;
} subs[MAX_SUBS];
int sub_count = 0;
int msqid;

void broker_cleanup(int sig) {
    for(int i=0; i<sub_count; i++) kill(subs[i].pid, SIGINT);
    msgctl(msqid, IPC_RMID, NULL);
    printf("\nБрокер завершил работу, очередь удалена.\n");
    exit(0);
}

void run_broker() {
    msqid = msgget(MSG_KEY, IPC_CREAT | IPC_EXCL | 0666);
    if (msqid < 0) {
        if(errno == EEXIST) printf("Ошибка: Очередь уже существует (другой брокер запущен).\n");
        else perror("msgget");
        exit(1);
    }
    signal(SIGINT, broker_cleanup);
    printf("Брокер запущен. Очередь ID: %d\n", msqid);

    struct msgbuf msg;
    while (1) {
        if (msgrcv(msqid, &msg, MAX_MSG, 1, 0) < 0) continue;

        if (strncmp(msg.mtext, "subscribe", 9) == 0) {
            int pid; char topic[50];
            sscanf(msg.mtext, "subscribe,%d,%s", &pid, topic);
            int found = -1;
            for(int i=0; i<sub_count; i++) if(subs[i].pid == pid) { found = i; break; }
            if (found == -1) {
                found = sub_count++;
                subs[found].pid = pid;
                subs[found].topic_count = 0;
            }
            strcpy(subs[found].topics[subs[found].topic_count++], topic);
        } 
        else if (strncmp(msg.mtext, "unsubscribe", 11) == 0) {
            int pid; char topic[50];
            sscanf(msg.mtext, "unsubscribe,%d,%s", &pid, topic);
            for(int i=0; i<sub_count; i++) {
                if(subs[i].pid == pid) {
                    for(int j=0; j<subs[i].topic_count; j++) {
                        if(strcmp(subs[i].topics[j], topic) == 0) {
                            subs[i].topics[j][0] = '\0'; // Помечаем как удаленную
                        }
                    }
                }
            }
        }
        else if (strncmp(msg.mtext, "send", 4) == 0) {
            char topic[50];
            int pid;
            sscanf(msg.mtext, "send,%d,%[^,]", &pid, topic);
            
            for(int i=0; i<sub_count; i++) {
                for(int j=0; j<subs[i].topic_count; j++) {
                    if(strcmp(subs[i].topics[j], topic) == 0) {
                        msg.mtype = subs[i].pid;
                        msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);
                        break;
                    }
                }
            }
        }
    }
}

void run_publisher(char *topic, char *text) {
    msqid = msgget(MSG_KEY, 0666);
    if (msqid < 0) { perror("Брокер не запущен"); exit(1); }
    struct msgbuf msg;
    msg.mtype = 1;
    snprintf(msg.mtext, MAX_MSG, "send,%d,%s,%s", getpid(), topic, text);
    msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);
    printf("Сообщение отправлено брокеру.\n");
}

void run_subscriber(char **topics, int tcount) {
    msqid = msgget(MSG_KEY, 0666);
    if (msqid < 0) { perror("Брокер не запущен"); exit(1); }
    
    struct msgbuf msg;
    for(int i=0; i<tcount; i++) {
        msg.mtype = 1;
        snprintf(msg.mtext, MAX_MSG, "subscribe,%d,%s", getpid(), topics[i]);
        msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0);
    }
    printf("Подписчик %d подписался на темы.\n", getpid());

    while (1) {
        if (msgrcv(msqid, &msg, MAX_MSG, getpid(), 0) < 0) {
            if(errno == EIDRM) break; // Очередь удалена
            continue;
        }
        printf("[Получено] %s\n", msg.mtext);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    if (strcmp(argv[1], "-b") == 0) run_broker();
    else if (strcmp(argv[1], "-p") == 0 && argc >= 4) run_publisher(argv[2], argv[3]);
    else if (strcmp(argv[1], "-s") == 0 && argc >= 3) run_subscriber(argv+2, argc-2);
    return 0;
}