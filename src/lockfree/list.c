#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "list.h"

struct node {
    val_t data;
    struct node *next;
};

struct list {
    node_t *head, *tail;
    uint32_t size;
};

/* The following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked,
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */
static inline bool is_marked_ref(void *i)
{
    return (bool) ((uintptr_t) i & 0x1L);
}

static inline void *get_unmarked_ref(void *w)
{
    return (void *) ((uintptr_t) w & ~0x1L);
}

static inline void *get_marked_ref(void *w)
{
    return (void *) ((uintptr_t) w | 0x1L);
}

/* list_search looks for value val, it
 *  - returns right_node owning val (if present) or its immediately higher
 *    value present in the list (otherwise) and
 *  - sets the left_node to the node owning the value immediately lower than
 *    val.
 * Encountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
static node_t *list_search(list_t *set, val_t val, node_t **left_node)
{
    node_t *left_node_next, *right_node;
    left_node_next = right_node = NULL;
    while (1) {
        node_t *t = set->head;
        node_t *t_next = set->head->next;
        while (is_marked_ref(t_next) || (t->data < val)) {
            if (!is_marked_ref(t_next)) {
                (*left_node) = t;
                left_node_next = t_next;
            }
            t = get_unmarked_ref(t_next);
            if (t == set->tail)
                break;
            t_next = t->next;
        }
        right_node = t;

        if (left_node_next == right_node) {
            if (!is_marked_ref(right_node->next))
                return right_node;
        } else {
            if (CAS_PTR(&((*left_node)->next), left_node_next, right_node) ==
                left_node_next) {
                if (!is_marked_ref(right_node->next))
                    return right_node;
            }
        }
    }
}

/* return true if there is a node in the list owning value val. */
bool list_contains(list_t *the_list, val_t val)
{
    node_t *iterator = get_unmarked_ref(the_list->head->next);
    while (iterator != the_list->tail) {
        if (!is_marked_ref(iterator->next) && iterator->data >= val) {
            /* either we found it, or found the first larger element */
            return iterator->data == val;
        }

        /* always get unmarked pointer */
        iterator = get_unmarked_ref(iterator->next);
    }
    return false;
}

static node_t *new_node(val_t val, node_t *next)
{
    node_t *node = malloc(sizeof(node_t));
    node->data = val;
    node->next = next;
    return node;
}

list_t *list_new()
{
    /* allocate list */
    list_t *the_list = malloc(sizeof(list_t));

    /* now need to create the sentinel node */
    the_list->head = new_node(INT_MIN, NULL);
    the_list->tail = new_node(INT_MAX, NULL);
    the_list->head->next = the_list->tail;
    the_list->size = 0;
    return the_list;
}

void list_delete(list_t *the_list)
{
    /* FIXME: implement the deletion */
}

int list_size(list_t *the_list)
{
    return the_list->size;
}

bool list_add(list_t *the_list, val_t val)
{
    node_t *left = NULL;
    node_t *new_elem = new_node(val, NULL);
    while (1) {
        node_t *right = list_search(the_list, val, &left);
        if (right != the_list->tail && right->data == val)
            return false;

        new_elem->next = right;
        if (CAS_PTR(&(left->next), right, new_elem) == right) {
            FAI_U32(&(the_list->size));
            return true;
        }
    }
}

/* The deletion is logical and consists of setting the node mark bit to 1. */
bool list_remove(list_t *the_list, val_t val)
{
    node_t *left = NULL;
    while (1) {
        node_t *right = list_search(the_list, val, &left);
        /* check if we found our node */
        if ((right == the_list->tail) || (right->data != val))
            return false;

        node_t *right_succ = right->next;
        if (!is_marked_ref(right_succ)) {
            if (CAS_PTR(&(right->next), right_succ,
                        get_marked_ref(right_succ)) == right_succ) {
                FAD_U32(&(the_list->size));
                return true;
            }
        }
    }
}
