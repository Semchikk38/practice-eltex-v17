#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_STR 50

typedef struct {
    char surname[MAX_STR];
    char name[MAX_STR];
    char patronymic[MAX_STR];
    char job[MAX_STR];
    char position[MAX_STR];
    char phones[3][MAX_STR];
    int phone_count;
    char emails[2][MAX_STR];
    int email_count;
    char social[2][MAX_STR];
    int social_count;
    char messengers[2][MAX_STR];
    int messenger_count;
} Contact;

typedef struct Node {
    Contact data;
    struct Node *prev;
    struct Node *next;
} Node;

extern Node *head;
void insert_sorted(Contact *c);
Node* find_node(const char *surname, const char *name);
void delete_node(Node *node);
void free_list(void);
void list_print(void);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 4.1 (dvyhsvyaznyy spisok) ===\n");
    free_list();

    Contact c1 = {"Alekseev","Aleksey","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c2 = {"Borisov","Boris","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c3 = {"Andreev","Andrey","","","",{""},0,{""},0,{""},0,{""},0};

    insert_sorted(&c1);
    insert_sorted(&c2);
    insert_sorted(&c3);

    printf("Spisok posle dobavleniya (dolzhen byt': Alekseev, Andreev, Borisov):\n");
    list_print();

    Node *cur = head;
    int ok = 1;
    if (strcmp(cur->data.surname, "Alekseev") != 0) ok = 0;
    cur = cur->next;
    if (cur == NULL || strcmp(cur->data.surname, "Andreev") != 0) ok = 0;
    cur = cur->next;
    if (cur == NULL || strcmp(cur->data.surname, "Borisov") != 0) ok = 0;
    if (ok) printf("Poryadok veren.\n");
    else printf("Poryadok narushen!\n");

    Node *node = find_node("Andreev", "Andrey");
    if (node) delete_node(node);
    printf("\nPosle udaleniya Andreeva:\n");
    list_print();

    int count = 0;
    cur = head;
    while (cur) { count++; cur = cur->next; }
    if (count == 2) printf("Kolichestvo elementov verno (2).\n");
    else printf("Oshibka: ozhidalos' 2, polucheno %d\n", count);

    free_list();
    printf("=== TEST ZAVERSHEN ===\n");
}