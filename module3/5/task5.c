#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/task5_shm"
#define BLOCKS_COUNT 50
#define DATA_SIZE 10

struct Block {
    int count;
    int next;
    int data[DATA_SIZE];
};

struct SharedMem {
    sem_t sem;
    int head;
    int free_head;
    int prod_done;
    struct Block blocks[BLOCKS_COUNT];
} *shm;

void init_shm() {
    sem_init(&shm->sem, 1, 1);
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
        sem_wait(&shm->sem);
        if (shm->free_head == -1) { sem_post(&shm->sem); sleep(1); continue; }
        if (generated >= 10) { shm->prod_done = 1; sem_post(&shm->sem); break; }

        int idx = shm->free_head;
        shm->free_head = shm->blocks[idx].next;

        int cnt = rand() % DATA_SIZE + 1;
        shm->blocks[idx].count = cnt;
        for(int i=0; i<cnt; i++) shm->blocks[idx].data[i] = rand() % 100;
        
        shm->blocks[idx].next = shm->head;
        shm->head = idx;
        generated++;
        sem_post(&shm->sem);
        printf("Producer: generated %d nums\n", cnt);
        sleep(1);
    }
}

void consumer() {
    while (1) {
        sem_wait(&shm->sem);
        if (shm->head == -1) {
            if (shm->prod_done) { sem_post(&shm->sem); break; }
            sem_post(&shm->sem); sleep(1); continue;
        }

        int idx = shm->head;
        shm->head = shm->blocks[idx].next;

        int min = shm->blocks[idx].data[0], max = min;
        for(int i=1; i<shm->blocks[idx].count; i++) {
            if(shm->blocks[idx].data[i] < min) min = shm->blocks[idx].data[i];
            if(shm->blocks[idx].data[i] > max) max = shm->blocks[idx].data[i];
        }
        printf("Consumer %d: Min=%d, Max=%d\n", getpid(), min, max);
        
        shm->blocks[idx].count = 0;
        shm->blocks[idx].next = shm->free_head;
        shm->free_head = idx;
        sem_post(&shm->sem);
        sleep(2);
    }
}

int main(int argc, char *argv[]) {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(struct SharedMem));
    shm = mmap(NULL, sizeof(struct SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (argc > 1 && strcmp(argv[1], "init") == 0) { init_shm(); return 0; }
    if (argc > 1 && strcmp(argv[1], "prod") == 0) producer();
    else if (argc > 1 && strcmp(argv[1], "cons") == 0) consumer();
    else if (argc > 1 && strcmp(argv[1], "clean") == 0) { shm_unlink(SHM_NAME); return 0; }

    munmap(shm, sizeof(struct SharedMem));
    close(fd);
    return 0;
}