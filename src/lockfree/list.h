#ifndef LLIST_H_
#define LLIST_H_

#include "atomic_ops_if.h"

#ifdef DEBUG
#define IO_FLUSH fflush(NULL)
#endif

typedef intptr_t val_t;

typedef struct node {
    val_t data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
    node_t *tail;
    uint32_t size;
} list_t;

list_t *list_new();

// return 0 if not found, positive number otherwise
int list_contains(list_t *the_list, val_t val);

// return 0 if value already in the list, positive number otherwise
int list_add(list_t *the_list, val_t val);

// return 0 if value already in the list, positive number otherwise
int list_remove(list_t *the_list, val_t val);
void list_delete(list_t *the_list);
int list_size(list_t *the_list);

node_t *list_search(list_t *the_list, val_t val, node_t **left_node);

#endif
