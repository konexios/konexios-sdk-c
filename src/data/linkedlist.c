#include "data/linkedlist.h"

linked_list_t *linked_list_add(linked_list_t *root, linked_list_t *el) {
    linked_list_t *last = root;
    el->next = 0;
    while ( last && last->next ) {
        last = last->next;
    }
    if ( last ) last->next = el;
    else root = el;
    return root;
}

linked_list_t *linked_list_add_first(linked_list_t *root, linked_list_t *el) {
    el->next = root;
    return el;
}

linked_list_t *linked_list_del(linked_list_t *root, linked_list_t *el) {
    linked_list_t *last = root;
    linked_list_t *prev = 0;
    while ( last ) {
        if ( last == el) {
            if ( prev ) {
                prev->next = last->next;
                last = 0;
                return root;
            } else {
                last = root->next;
                root->next = 0;
                return last;
            }
        }
        prev = last;
        last = last->next;
    }
    return 0;
}

linked_list_t *linked_list_del_last(linked_list_t *root) {
    linked_list_t *last = root;
    linked_list_t *prev = 0;
    while ( last && last->next ) {
        prev = last;
        last = last->next;
    }
    if ( prev ) {
        prev->next = last->next;
        last->next = 0;
        return root;
    } 
    return 0;
}
