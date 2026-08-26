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

typedef struct TreeNode {
    Contact data;
    struct TreeNode *left;
    struct TreeNode *right;
    int height;
} TreeNode;

extern TreeNode *root;
TreeNode* insert_node(TreeNode *node, Contact *c);
TreeNode* delete_node(TreeNode *node, const char *surname, const char *name);
TreeNode* find_node(TreeNode *node, const char *surname, const char *name);
void inorder(TreeNode *node);
void free_tree(TreeNode *node);

void run_tests(void) {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    printf("\n=== TEST 4.3 (AVL-derevo) ===\n");
    free_tree(root);
    root = NULL;

    Contact c1 = {"Ivanov","Ivan","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c2 = {"Petrov","Petr","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c3 = {"Sidorov","Sidor","","","",{""},0,{""},0,{""},0,{""},0};
    Contact c4 = {"Alekseev","Aleksey","","","",{""},0,{""},0,{""},0,{""},0};

    root = insert_node(root, &c1);
    root = insert_node(root, &c2);
    root = insert_node(root, &c3);
    root = insert_node(root, &c4);

    printf("Derevo (simmetrichnyy obhod, dolzhen byt' po alfavitu):\n");
    inorder(root);

    root = delete_node(root, "Petrov", "Petr");
    printf("\nPosle udaleniya Petrova:\n");
    inorder(root);

    if (find_node(root, "Ivanov", "Ivan") &&
        find_node(root, "Sidorov", "Sidor") &&
        find_node(root, "Alekseev", "Aleksey")) {
        printf("Vse ostavshiesya kontakty naydeny.\n");
    } else {
        printf("Oshibka: ne vse kontakty naydeny.\n");
    }

    free_tree(root);
    root = NULL;
    printf("=== TEST ZAVERSHEN ===\n");
}