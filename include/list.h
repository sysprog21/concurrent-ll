/* interface for the list */

#ifndef _LIST_H_
#define _LIST_H_

#include "lock_if.h"

typedef intptr_t val_t;

typedef struct node node_t;
typedef struct list list_t;

/* return 0 if not found, positive number otherwise */
list_t *list_new();

/* return 0 if value already in the list, positive number otherwise */
int list_contains(list_t *the_list, val_t val);

int list_add(list_t *the_list, val_t val);
int list_remove(list_t *the_list, val_t val);
void list_delete(list_t *the_list);
int list_size(list_t *the_list);

#endif
