#include "data/linkedlist.h"

konexios_linked_list_t *konexios_linked_list_add(konexios_linked_list_t *root, konexios_linked_list_t *el) {
    konexios_linked_list_t *last = root;
    el->next = 0;
    while ( last && last->next ) {
        last = last->next;
    }
    if ( last ) last->next = el;
    else root = el;
    return root;
}

konexios_linked_list_t *konexios_linked_list_add_first(konexios_linked_list_t *root, konexios_linked_list_t *el) {
    el->next = root;
    return el;
}

konexios_linked_list_t *konexios_linked_list_del(konexios_linked_list_t *root, konexios_linked_list_t *el) {
    konexios_linked_list_t *last = root;
    konexios_linked_list_t *prev = 0;
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

konexios_linked_list_t *konexios_linked_list_del_last(konexios_linked_list_t *root) {
    konexios_linked_list_t *last = root;
    konexios_linked_list_t *prev = 0;
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
