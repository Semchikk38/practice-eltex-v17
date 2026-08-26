#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "list.h"
#include "contact.h"

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 6.1 (staticheskaya biblioteka) ===\n");
    free_list();

    Contact c1 = {"Alekseev","Aleksey","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c2 = {"Borisov","Boris","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c3 = {"Andreev","Andrey","","","",{""},0,{""},0,{""},0,{""},0};

    insert_sorted(&c1);
    insert_sorted(&c2);
    insert_sorted(&c3);

    printf("Spisok (dolzhen byt': Alekseev, Andreev, Borisov):\n");
    list_print();

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
    Node *cur = NULL;

    free_list();
    printf("=== TEST ZAVERSHEN ===\n");
}