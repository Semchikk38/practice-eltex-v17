#ifndef LIST_H
#define LIST_H

#include "contact.h"

typedef struct Node {
    Contact data;
    struct Node *prev;
    struct Node *next;
} Node;

void insert_sorted(Contact *c);
Node* find_node(const char *surname, const char *name);
void delete_node(Node *node);
void free_list(void);
void list_print(void);
Node* get_head(void);

#endif