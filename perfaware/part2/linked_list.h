#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

typedef struct list_node {
    void *data;
    struct list_node *next;
} list_node;

typedef struct linked_list{
    list_node *first;
    list_node *last;
} linked_list;

linked_list list_create(void);
list_node* list_push_back(linked_list *l);
list_node* list_pop_front(linked_list *l);
bool list_delete_front(linked_list *l, bool free_data);

// Here is a function to see this linked list in action.
void linked_list_demo(void);

#endif