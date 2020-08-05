#ifndef FAST_FRAMEDATA_H
#define FAST_FRAMEDATA_H

#include "page_table.h"

// Intrusive linked list implementation

struct list_node_t {
    struct list_node_t *next, *prev;
};

typedef struct list_node_t os_list_node;

static os_list_node head_arr[1<<PAGE_BITS];

static void os_list_init() {
    for (int i=0; i!=(1<<PAGE_BITS); ++i) {
        os_list_node* const curr = head_arr + i;
        curr->next = curr;
        curr->prev = curr;
    }
}

// associates this os_list_node with this page_index
static void os_register_list_node(os_list_node* node, int page_index) {
    os_list_node* const head = head_arr + page_index;
    os_list_node* const aft = head->next;
    head->next = node;
    node->prev = head;
    aft->prev = node;
    node->next = aft;
}

// de-associates this os_list_node from this page_index
static void os_deregister_list_node(os_list_node* node) {
    os_list_node* const bef = node->prev;
    os_list_node* const aft = node->next;
    bef->next = aft;
    aft->prev = bef;
}

static os_list_node const* os_list_end_nullify(int page_index, os_list_node const* node) {
    if (node == head_arr + page_index) return NULL;
    return node;
}

// gets the head of the list associated with this page_index
// will be NULL if this list has no nodes
static os_list_node const* os_get_list(int page_index) {
    return os_list_end_nullify(page_index, head_arr[page_index].next);
}

// check if this os_list_node is the empty sentinel node
// will return NULL if this node is the last node
static os_list_node const* os_list_next(int page_index, os_list_node const* node) {
    return os_list_end_nullify(page_index, node->next);
}

// de-associates all list_nodes from this page_index
static void os_clear_list(int page_index) {
    os_list_node* const curr = head_arr + page_index;
    curr->next = curr;
    curr->prev = curr;
}

#endif
