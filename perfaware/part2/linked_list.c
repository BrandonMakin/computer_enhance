#include <stdbool.h>
#include <stdio.h>

#include "linked_list.h"

linked_list list_create(void)
{
    linked_list list = {NULL, NULL};
    return list;
}

list_node* list_push_back(linked_list *l)
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
    return node;
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
    list_node *current = list_push_back(&list);
    current->data = "I'm";
    current = list_push_back(&list);
    current->data = "headed";
    current = list_push_back(&list);
    current->data = "straight";
    current = list_push_back(&list);
    current->data = "for";
    current = list_push_back(&list);
    current->data = "the";
    current = list_push_back(&list);
    current->data = "floor.";

    current = list.first;
    while (current != NULL)
    {
        printf("%s ", (char*)current->data);
        current = current->next;
    }
    puts("");
}