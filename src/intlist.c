#include <stdlib.h>
#include <stdio.h>

typedef struct IntListNode {
    int value;
    struct IntListNode* next;
} IntListNode;

typedef struct {
    int length;
    IntListNode* node;
} IntList;

IntListNode* create_list_node(const int value) {
    IntListNode* node = malloc(sizeof(IntListNode));

    node->value = value;
    node->next = NULL;

    return node;
}

IntList* new_list() {
    // holds linked list metadata and pointer to the first node
    IntList* list = malloc(sizeof(IntList));

    list->node = NULL;
    list->length = -1;

    return list;
}


void append_list(IntList* list, int value) {
    // handle empty list
    if (list->node == NULL) {
        list->node = create_list_node(value);
        list->length += 1;
        return;
    }

    // first node
    IntListNode* current_node = list->node;
    // following node (NULL if there's only one node)
    IntListNode* next_node = current_node->next;

    // traverse nodes until the end is reached
    while (next_node != NULL) {
        current_node = next_node;
        next_node = current_node->next;
    }
    
    current_node->next = create_list_node(value);
    list->length += 1;
}

void print_list(const IntList* list) {
    const IntListNode* current_node = list->node;
    
    for (int i = 0; i < list->length; i++) {
        printf("%i\n", current_node->value);
        current_node = current_node->next;
    }
}