#include <stdbool.h>
#include <stdio.h>

#include "linked_list.h"

linked_list list_create(void)
{
    linked_list list = {NULL, NULL};
    return list;
}

void list_push_back(linked_list *l, void *data)
{
    list_node *node = (list_node*)calloc(1, sizeof(node));
    if (l->first == NULL)
    {
        l->first = node;
    }
    else
    {
        l->last->next = node;
    }
    l->last = node;
    node->data = data;
}

list_node* list_pop_front(linked_list *l)
{
    if (l->first == NULL) // empty linked_list
    {
        return NULL;
    }
    else if (l->first == l->last) // linked_list has only 1 node
    {
        list_node *node = l->first;
        l->first = NULL;
        l->last = NULL;
        return node;
    }
    else // linked_list has multiple nodes
    {
        list_node *node = l->first;
        l->first = l->first->next;
        return node;
    }
}

bool list_delete_front(linked_list *l, bool free_data)
{
    list_node *node = list_pop_front(l);
    if (node == NULL)
    {
        return false;
    }
    else
    {
        if (free_data)
        {
            free(node->data);
        }
        free(node);
        // we don't need to set node to NULL b/c node is contained to this scope
        return true;
    }
}

void linked_list_demo(void)
{
    linked_list list = list_create();
    list_push_back(&list, "I'm");
    list_push_back(&list, "headed");
    list_push_back(&list, "straight");
    list_push_back(&list, "for");
    list_push_back(&list, "the");
    list_push_back(&list, "floor.");

    list_node *current = list.first;
    while (current != NULL)
    {
        printf("%s ", (char*)current->data);
        current = current->next;
    }
    puts("");
}