#include "atomic_ops_if.h"
#include "list.h"

int list_contains(llist_t *the_list, val_t val)
{
    /* lock sentinel node */
    node_t *elem = the_list->head;
    LOCK(elem->lock);
    if (elem->next == NULL) {
        // the list is empty
        UNLOCK(elem->lock);
        return 0;
    }

    node_t *prev = elem;

    while (elem->next != NULL && elem->next->data <= val) {
        if (elem->next->data == val) {
            /* found it, return success */
            UNLOCK(elem->lock);
            return 1;
        }
        prev = elem;
        elem = elem->next;
        LOCK(elem->lock);
        UNLOCK(prev->lock);
    }

    /* just check if the last node in the list is not equal to val */
    if (elem->data == val) {
        UNLOCK(elem->lock);
        // we found it
        return 1;
    }

    // not found in the list
    UNLOCK(elem->lock);
    return 0;
}

static node_t *new_node(val_t val, node_t *next)
{
    /* allocate node */
    node_t *node = malloc(sizeof(node_t));
    /* allocate lock */
    node->lock = malloc(sizeof(ptlock_t));
    /* let's initialize the lock */
    INIT_LOCK(node->lock);

    node->data = val;
    node->next = next;
    return node;
}

llist_t *list_new()
{
    /* allocate list */
    llist_t *the_list = malloc(sizeof(llist_t));

    /* now need to create the sentinel node */
    the_list->head = new_node(0, NULL);
    return the_list;
}

void list_delete(llist_t *the_list)
{
    /* must lock the whole list */
    node_t *elem = the_list->head;
    LOCK(elem->lock);
    if (elem->next == NULL) {
        /* we have an empty list, just delete sentinel node */
        UNLOCK(elem->lock);
        DESTROY_LOCK(elem->lock);

        /* deallocate memory and we are done */
        free(elem->lock);
        free(elem);
    } else {
        // we need to go through list
        while (elem->next != NULL) {
            // lock everything
            LOCK(elem->next->lock);
            elem = elem->next;
        }

        // everything is locked, delete them
        while (the_list->head != NULL) {
            elem = the_list->head;
            the_list->head = elem->next;

            UNLOCK(elem->lock);
            DESTROY_LOCK(elem->lock);

            free(elem->lock);
            free(elem);
        }
    }

    // deallocate memory
    free(the_list);
}

int list_size(llist_t *the_list)
{
    int size = 0;
    // must lock the whole list
    node_t *prev = the_list->head;
    LOCK(prev->lock);
    if (prev->next == NULL) {
        // the list is empty
        UNLOCK(prev->lock);
        return size;
    }

    node_t *elem = prev->next;
    LOCK(elem->lock);
    size++;
    while (elem->next != NULL) {
        size++;
        UNLOCK(prev->lock);
        prev = elem;
        elem = elem->next;
        LOCK(elem->lock);
    }

    // we did not find it; unlock and report failure
    UNLOCK(elem->lock);
    UNLOCK(prev->lock);
    return size;
}

int list_add(llist_t *the_list, val_t val)
{
    // lock sentinel node
    node_t *elem = the_list->head;
    LOCK(elem->lock);
    if (elem->next == NULL) {
        // the list is empty
        node_t *newElem = new_node(val, NULL);
        elem->next = newElem;
        UNLOCK(elem->lock);
        return 1;
    }

    node_t *prev = elem;

    while (elem->next != NULL && elem->next->data <= val) {
        if (elem->next->data == val) {
            // we already have that value, unlock and report failure
            UNLOCK(elem->lock);
            return 0;
        }
        prev = elem;
        elem = elem->next;
        LOCK(elem->lock);
        UNLOCK(prev->lock);
    }
    // just check if the last node in the list is not equal to val
    if (elem->data == val) {
        UNLOCK(elem->lock);
        // if equal report failure
        return 0;
    }

    // place it in between prev and elem
    node_t *newElem = new_node(val, elem->next);
    elem->next = newElem;

    // successfully added new value, unlock  elem
    UNLOCK(elem->lock);
    return 1;
}

int list_remove(llist_t *the_list, val_t val)
{
    // lock sentinel node
    node_t *prev = the_list->head;
    LOCK(prev->lock);
    if (prev->next == NULL) {
        // the list is empty
        UNLOCK(prev->lock);
        return 0;
    }

    node_t *elem = prev->next;
    LOCK(elem->lock);
    while (elem->next != NULL && elem->data <= val) {
        if (elem->data == val) {
            // if found, assign prev next to elem next
            prev->next = elem->next;

            // unlock and deallocate mem
            UNLOCK(elem->lock);
            DESTROY_LOCK(elem->lock);
            free(elem->lock);
            free(elem);
            // its a success
            UNLOCK(prev->lock);
            return 1;
        }
        UNLOCK(prev->lock);
        prev = elem;
        elem = elem->next;
        LOCK(elem->lock);
    }
    // just check if the last node in the list is not equal to val
    if (elem->data == val) {
        // if found, assign prev next to elem next
        prev->next = elem->next;

        // unlock and deallocate mem
        UNLOCK(elem->lock);
        DESTROY_LOCK(elem->lock);
        free(elem->lock);
        free(elem);
        // its a success
        UNLOCK(prev->lock);
        return 1;
    }

    // we did not find it; unlock and report failure
    UNLOCK(elem->lock);
    UNLOCK(prev->lock);
    return 0;
}
