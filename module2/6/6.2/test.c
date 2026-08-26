#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "list.h"
#include "contact.h"

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 6.2 (dinamicheskaya biblioteka) ===\n");
    free_list();

    Contact c1 = {"Alekseev","Aleksey","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c2 = {"Borisov","Boris","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c3 = {"Andreev","Andrey","","","",{""},0,{""},0,{""},0,{""},0};

    insert_sorted(&c1);
    insert_sorted(&c2);
    insert_sorted(&c3);

    printf("Spisok posle dobavleniya (dolzhen byt': Alekseev, Andreev, Borisov):\n");
    list_print();

    // Proverka poryadka
    Node *cur = get_head();
    int ok = 1;
    if (cur == NULL || strcmp(cur->data.surname, "Alekseev") != 0) ok = 0;
    if (ok && cur->next != NULL) {
        cur = cur->next;
        if (strcmp(cur->data.surname, "Andreev") != 0) ok = 0;
    } else ok = 0;
    if (ok && cur->next != NULL) {
        cur = cur->next;
        if (strcmp(cur->data.surname, "Borisov") != 0) ok = 0;
    } else ok = 0;
    
    if (ok) printf("Poryadok veren.\n");
    else printf("Poryadok narushen!\n");

    // Udalenie Andreeva
    Node *node = find_node("Andreev", "Andrey");
    if (node) {
        delete_node(node);
        printf("\nPosle udaleniya Andreeva:\n");
        list_print();
    } else {
        printf("\nAndreev ne nayden!\n");
    }

    // Proverka kolichestva
    int count = 0;
    cur = get_head();
    while (cur) { count++; cur = cur->next; }
    if (count == 2) printf("Kolichestvo elementov verno (2).\n");
    else printf("Oshibka: ozhidalos' 2, polucheno %d\n", count);

    free_list();
    printf("=== TEST ZAVERSHEN ===\n");
}