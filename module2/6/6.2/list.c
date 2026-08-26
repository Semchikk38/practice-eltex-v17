#include "list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static Node *head = NULL;

static int compare_contact(const Contact *a, const Contact *b) {
    int cmp = strcmp(a->surname, b->surname);
    if (cmp != 0) return cmp;
    return strcmp(a->name, b->name);
}

static Node* create_node(const Contact *c) {
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

Node* get_head(void) {
    return head;
}