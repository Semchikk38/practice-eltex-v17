#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_MSG 100

typedef struct QueueItem {
    int priority;
    char message[MAX_MSG];
    struct QueueItem *next;
} QueueItem;

QueueItem *head = NULL;

// Prototip funktsii testov
void run_tests(void);

void enqueue(int priority, const char *msg) {
    QueueItem *new_item = (QueueItem*)malloc(sizeof(QueueItem));
    if (!new_item) {
        printf("Oshibka pamyati\n");
        return;
    }
    new_item->priority = priority;
    strcpy(new_item->message, msg);
    new_item->next = NULL;
    if (head == NULL || priority > head->priority) {
        new_item->next = head;
        head = new_item;
        return;
    }
    QueueItem *cur = head;
    while (cur->next != NULL && cur->next->priority >= priority) {
        cur = cur->next;
    }
    new_item->next = cur->next;
    cur->next = new_item;
}

QueueItem* dequeue_first(void) {
    if (head == NULL) return NULL;
    QueueItem *item = head;
    head = head->next;
    item->next = NULL;
    return item;
}

QueueItem* dequeue_by_priority(int priority) {
    if (head == NULL) return NULL;
    if (head->priority == priority) return dequeue_first();
    QueueItem *cur = head;
    while (cur->next != NULL && cur->next->priority != priority) {
        cur = cur->next;
    }
    if (cur->next == NULL) return NULL;
    QueueItem *item = cur->next;
    cur->next = item->next;
    item->next = NULL;
    return item;
}

QueueItem* dequeue_by_min_priority(int threshold) {
    if (head == NULL) return NULL;
    if (head->priority >= threshold) return dequeue_first();
    QueueItem *cur = head;
    while (cur->next != NULL && cur->next->priority < threshold) {
        cur = cur->next;
    }
    if (cur->next == NULL) return NULL;
    QueueItem *item = cur->next;
    cur->next = item->next;
    item->next = NULL;
    return item;
}

void free_queue(void) {
    while (head != NULL) {
        QueueItem *tmp = head;
        head = head->next;
        free(tmp);
    }
}

void print_queue(void) {
    QueueItem *cur = head;
    if (cur == NULL) {
        printf("Ochered' pusta.\n");
        return;
    }
    while (cur) {
        printf("[%d] %s\n", cur->priority, cur->message);
        cur = cur->next;
    }
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }
    
    printf("Zapustite ./main --test dlya avtomaticheskoy proverki.\n");
    return 0;
}