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

// Prototipy funktsiy iz main.c
void enqueue(int priority, const char *msg);
QueueItem* dequeue_first(void);
QueueItem* dequeue_by_priority(int priority);
QueueItem* dequeue_by_min_priority(int threshold);
void free_queue(void);
void print_queue(void);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 4.2 (ochered' s prioritetom) ===\n");
    
    // Ochishchaem ochered' na vsyakiy sluchay
    free_queue();
    
    enqueue(10, "Nizkiy");
    enqueue(50, "Sredniy");
    enqueue(200, "Vysokiy");
    enqueue(150, "Pochti vysokiy");
    enqueue(30, "Nizhe srednego");

    printf("Ishodnaya ochered' (po ubyvaniyu prioriteta):\n");
    print_queue();

    QueueItem *item = dequeue_first();
    if (item) {
        printf("\nIzvlechen pervyy: prioritet %d, '%s'\n", item->priority, item->message);
        free(item);
    }
    printf("Ochered' posle izvlecheniya:\n");
    print_queue();

    item = dequeue_by_priority(50);
    if (item) {
        printf("\nIzvlechen s prioritetom 50: '%s'\n", item->message);
        free(item);
    } else {
        printf("Ne nayden prioritet 50\n");
    }
    printf("Ochered' posle izvlecheniya 50:\n");
    print_queue();

    item = dequeue_by_min_priority(100);
    if (item) {
        printf("\nIzvlechen s prioritetom >=100: prioritet %d, '%s'\n", item->priority, item->message);
        free(item);
    } else {
        printf("Net elementa s prioritetom >=100\n");
    }
    printf("Ochered' posle izvlecheniya >=100:\n");
    print_queue();

    free_queue();
    printf("Ochered' ochishchena.\n");
    printf("=== TEST ZAVERSHEN ===\n");
}