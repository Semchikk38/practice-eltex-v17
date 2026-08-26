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

typedef struct TreeNode {
    Contact data;
    struct TreeNode *left;
    struct TreeNode *right;
    int height;
} TreeNode;

TreeNode *root = NULL;

// Prototip funktsii testov
void run_tests(void);

int compare_contact(const Contact *a, const Contact *b) {
    int cmp = strcmp(a->surname, b->surname);
    if (cmp != 0) return cmp;
    return strcmp(a->name, b->name);
}

int height(TreeNode *node) { 
    return node ? node->height : 0; 
}

int max_int(int a, int b) { 
    return (a > b) ? a : b; 
}

TreeNode* rotate_right(TreeNode *y) {
    TreeNode *x = y->left;
    TreeNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(height(y->left), height(y->right)) + 1;
    x->height = max_int(height(x->left), height(x->right)) + 1;
    return x;
}

TreeNode* rotate_left(TreeNode *x) {
    TreeNode *y = x->right;
    TreeNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(height(x->left), height(x->right)) + 1;
    y->height = max_int(height(y->left), height(y->right)) + 1;
    return y;
}

TreeNode* balance(TreeNode *node) {
    if (node == NULL) return NULL;
    node->height = max_int(height(node->left), height(node->right)) + 1;
    int bal = height(node->left) - height(node->right);
    if (bal > 1) {
        if (height(node->left->left) >= height(node->left->right)) {
            return rotate_right(node);
        } else {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
    }
    if (bal < -1) {
        if (height(node->right->right) >= height(node->right->left)) {
            return rotate_left(node);
        } else {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
    }
    return node;
}

TreeNode* insert_node(TreeNode *node, Contact *c) {
    if (node == NULL) {
        TreeNode *new_node = (TreeNode*)malloc(sizeof(TreeNode));
        if (!new_node) {
            printf("Oshibka pamyati\n");
            exit(1);
        }
        new_node->data = *c;
        new_node->left = new_node->right = NULL;
        new_node->height = 1;
        return new_node;
    }
    int cmp = compare_contact(c, &node->data);
    if (cmp < 0) node->left = insert_node(node->left, c);
    else if (cmp > 0) node->right = insert_node(node->right, c);
    else return node;
    return balance(node);
}

TreeNode* find_node(TreeNode *node, const char *surname, const char *name) {
    if (node == NULL) return NULL;
    Contact temp;
    strcpy(temp.surname, surname);
    strcpy(temp.name, name);
    int cmp = compare_contact(&temp, &node->data);
    if (cmp < 0) return find_node(node->left, surname, name);
    else if (cmp > 0) return find_node(node->right, surname, name);
    else return node;
}

TreeNode* find_min(TreeNode *node) {
    while (node->left != NULL) node = node->left;
    return node;
}

TreeNode* delete_node(TreeNode *node, const char *surname, const char *name) {
    if (node == NULL) return NULL;
    Contact temp;
    strcpy(temp.surname, surname);
    strcpy(temp.name, name);
    int cmp = compare_contact(&temp, &node->data);
    if (cmp < 0) node->left = delete_node(node->left, surname, name);
    else if (cmp > 0) node->right = delete_node(node->right, surname, name);
    else {
        if (node->left == NULL || node->right == NULL) {
            TreeNode *child = node->left ? node->left : node->right;
            if (child == NULL) {
                free(node);
                return NULL;
            } else {
                *node = *child;
                free(child);
            }
        } else {
            TreeNode *min = find_min(node->right);
            node->data = min->data;
            node->right = delete_node(node->right, min->data.surname, min->data.name);
        }
    }
    return balance(node);
}

void inorder(TreeNode *node) {
    if (node == NULL) return;
    inorder(node->left);
    Contact *c = &node->data;
    printf("%s %s\n", c->surname, c->name);
    inorder(node->right);
}

void free_tree(TreeNode *node) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
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