#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>
#include <string.h>

#define SHM_KEY 54321
#define SEM_KEY 54322
#define BLOCKS_COUNT 50
#define DATA_SIZE 10

struct Block {
    int count; // >0: есть данные, 0: блок свободен
    int next;  // индекс следующего блока, -1 если конец
    int data[DATA_SIZE];
};

struct SharedMem {
    int head;       // Индекс первого блока с данными
    int free_head;  // Индекс первого свободного блока
    int prod_done;  // Флаг завершения производителя
    struct Block blocks[BLOCKS_COUNT];
} *shm;

int semid;
void sem_lock() { struct sembuf op = {0, -1, 0}; semop(semid, &op, 1); }
void sem_unlock() { struct sembuf op = {0, 1, 0}; semop(semid, &op, 1); }

void init_shm() {
    shm->head = -1;
    shm->free_head = 0;
    shm->prod_done = 0;
    for(int i=0; i<BLOCKS_COUNT; i++) {
        shm->blocks[i].count = 0;
        shm->blocks[i].next = (i == BLOCKS_COUNT - 1) ? -1 : i + 1;
    }
}

void producer() {
    srand(time(NULL));
    int generated = 0;
    while (1) {
        sem_lock();
        if (shm->free_head == -1) {
            sem_unlock();
            printf("Производитель: Память заполнена, жду...\n");
            sleep(1);
            continue;
        }
        if (generated >= 10) { // Условие завершения для теста
            shm->prod_done = 1;
            sem_unlock();
            break;
        }

        int idx = shm->free_head;
        shm->free_head = shm->blocks[idx].next;

        int cnt = rand() % DATA_SIZE + 1;
        shm->blocks[idx].count = cnt;
        for(int i=0; i<cnt; i++) shm->blocks[idx].data[i] = rand() % 100;
        
        shm->blocks[idx].next = shm->head;
        shm->head = idx;
        generated++;
        sem_unlock();
        printf("Производитель: Сгенерирован блок из %d чисел.\n", cnt);
        sleep(1);
    }
}

void consumer() {
    while (1) {
        sem_lock();
        if (shm->head == -1) {
            if (shm->prod_done) {
                sem_unlock();
                break; // Производитель закончил и данных нет
            }
            sem_unlock();
            sleep(1);
            continue;
        }

        int idx = shm->head;
        shm->head = shm->blocks[idx].next;

        int min = shm->blocks[idx].data[0], max = min;
        for(int i=1; i<shm->blocks[idx].count; i++) {
            if(shm->blocks[idx].data[i] < min) min = shm->blocks[idx].data[i];
            if(shm->blocks[idx].data[i] > max) max = shm->blocks[idx].data[i];
        }
        printf("Потребитель %d: Обработал блок. Min=%d, Max=%d\n", getpid(), min, max);
        
        shm->blocks[idx].count = 0;
        shm->blocks[idx].next = shm->free_head;
        shm->free_head = idx;
        sem_unlock();
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    int shmid = shmget(SHM_KEY, sizeof(struct SharedMem), IPC_CREAT | 0666);
    shm = shmat(shmid, NULL, 0);
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    if (argc > 1 && strcmp(argv[1], "init") == 0) {
        init_shm();
        printf("Память инициализирована.\n");
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "prod") == 0) producer();
    else consumer();

    shmdt(shm);
    return 0;
}