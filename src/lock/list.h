/* interface for the list */

#ifndef LLIST_H_
#define LLIST_H_

#include "lock_if.h"

#ifdef DEBUG
#define IO_FLUSH fflush(NULL)
#endif

typedef intptr_t val_t;

typedef struct node {
    val_t data;         // data
    struct node *next;  // pointer to the next entry
    ptlock_t *lock;     // lock for this entry
} node_t;

typedef struct {
    node_t *head;  // pointer to the head of the list
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

#endif
