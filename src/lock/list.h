/*
 *  linkedlist.h
 *  interface for the list
 *
 */
#ifndef LLIST_H_
#define LLIST_H_

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include "atomic_ops_if.h"
#include "lock_if.h"
#include "utils.h"

#ifdef DEBUG
#define IO_FLUSH fflush(NULL)
#endif

typedef intptr_t val_t;

typedef struct node {
    val_t data;         // data
    struct node *next;  // pointer to the next entry
    ptlock_t *lock;     // lock for this entry
} node_t;

typedef struct llist {
    node_t *head;  // pointer to the head of the list
} llist_t;

llist_t *list_new();
// return 0 if not found, positive number otherwise
int list_contains(llist_t *the_list, val_t val);
// return 0 if value already in the list, positive number otherwise
int list_add(llist_t *the_list, val_t val);
// return 0 if value already in the list, positive number otherwise
int list_remove(llist_t *the_list, val_t val);
void list_delete(llist_t *the_list);
int list_size(llist_t *the_list);

#endif
