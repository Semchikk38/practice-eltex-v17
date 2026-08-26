#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_STR 50
#define MAX_PHONES 3
#define MAX_EMAILS 2
#define MAX_SOCIAL 2
#define MAX_MESSENGERS 2

typedef struct {
    char surname[MAX_STR];
    char name[MAX_STR];
    char patronymic[MAX_STR];
    char job[MAX_STR];
    char position[MAX_STR];
    char phones[MAX_PHONES][MAX_STR];
    int phone_count;
    char emails[MAX_EMAILS][MAX_STR];
    int email_count;
    char social[MAX_SOCIAL][MAX_STR];
    int social_count;
    char messengers[MAX_MESSENGERS][MAX_STR];
    int messenger_count;
} Contact;

typedef struct Node {
    Contact data;
    struct Node *prev;
    struct Node *next;
} Node;

Node *head = NULL;

// Prototip funktsii testov (iz test.c)
void run_tests(void);

int compare_contact(const Contact *a, const Contact *b) {
    int cmp = strcmp(a->surname, b->surname);
    if (cmp != 0) return cmp;
    return strcmp(a->name, b->name);
}

Node* create_node(const Contact *c) {
    Node *new_node = (Node*)malloc(sizeof(Node));
    if (!new_node) {
        printf("Oshibka pamyati\n");
        exit(1);
    }
    new_node->data = *c;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

void insert_sorted(Contact *c) {
    Node *new_node = create_node(c);
    if (head == NULL) {
        head = new_node;
        return;
    }
    if (compare_contact(c, &head->data) < 0) {
        new_node->next = head;
        head->prev = new_node;
        head = new_node;
        return;
    }
    Node *cur = head;
    while (cur->next != NULL && compare_contact(c, &cur->next->data) > 0) {
        cur = cur->next;
    }
    new_node->next = cur->next;
    new_node->prev = cur;
    if (cur->next != NULL) cur->next->prev = new_node;
    cur->next = new_node;
}

Node* find_node(const char *surname, const char *name) {
    Node *cur = head;
    while (cur != NULL) {
        if (strcmp(cur->data.surname, surname) == 0 &&
            strcmp(cur->data.name, name) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void delete_node(Node *node) {
    if (node == NULL) return;
    if (node->prev != NULL) node->prev->next = node->next;
    else head = node->next;
    if (node->next != NULL) node->next->prev = node->prev;
    free(node);
}

void free_list(void) {
    Node *cur = head;
    while (cur != NULL) {
        Node *tmp = cur;
        cur = cur->next;
        free(tmp);
    }
    head = NULL;
}

void list_print(void) {
    if (head == NULL) {
        printf("Spisok pust.\n");
        return;
    }
    Node *cur = head;
    while (cur != NULL) {
        Contact *c = &cur->data;
        printf("%s %s\n", c->surname, c->name);
        cur = cur->next;
    }
}

int main(int argc, char *argv[]) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    
    // Proverka argumenta
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }
    
    printf("Zapustite ./main --test dlya avtomaticheskoy proverki.\n");
    return 0;
}