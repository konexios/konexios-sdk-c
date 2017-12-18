#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <data/dllist.h>

typedef struct _test_ {
  int data;
  int count;
  int hello;
  doubly_linked_list_head_node;
} test_t;

void setUp(void)
{
}

void tearDown(void)
{
}

static test_t *__root = NULL;

void test_create_doubly_linkedlist(void) {
    test_t *node = (test_t *)malloc(sizeof(test_t));
    node->data = 10;
    node->count = 0;
    node->hello = 111;
    doubly_linked_list_add_node(__root, test_t, node);
    TEST_ASSERT(__root);
    TEST_ASSERT_EQUAL_INT(10, __root->data);
    TEST_ASSERT_EQUAL_INT(0, __root->count);
    TEST_ASSERT_EQUAL_INT(111, __root->hello);
    TEST_ASSERT( __root->node.next );
}

void test_add_create_linkedlist(void) {
    test_t *node2 = (test_t *)malloc(sizeof(test_t));
    node2->data = 11;
    node2->count = 1;
    node2->hello = 836;
    doubly_linked_list_add_node_tail(__root, test_t, node2);
    TEST_ASSERT( __root->node.next );
    TEST_ASSERT( __root->node.next->next );
    TEST_ASSERT_EQUAL_INT(0, __root->count);

    test_t *tmp = container_of(__root->node.next, test_t, node);
    TEST_ASSERT_EQUAL_INT(1, tmp->count);

    // fill
    test_t *node3 = (test_t *)malloc(sizeof(test_t));
    node3->data = 12;
    node3->count = 2;
    node3->hello = 528;
    doubly_linked_list_add_node_tail(__root, test_t, node3);
    TEST_ASSERT_EQUAL_INT(0, __root->count);

    tmp = container_of(__root->node.next->next, test_t, node);
    TEST_ASSERT_EQUAL_INT(2, tmp->count);
}

void test_check_order(void) {
    int exp_ord[] = {1, 2, 0};
    test_t *tmp;
    int i = 0;
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
    }
}

void test_check_add_order_last(void) {
    int exp_ord[] = {1, 2, 0, 3};
    test_t *node4 = (test_t *)malloc(sizeof(test_t));
    node4->data = 13;
    node4->count = 3;
    node4->hello = 964;
    doubly_linked_list_add_node(__root, test_t, node4);
    test_t *tmp = NULL;
    test_t *tmp_last = NULL;
    int i = 0;
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
        tmp_last = tmp;
    }
    TEST_ASSERT(tmp_last);
    TEST_ASSERT_EQUAL_INT(13, tmp_last->data);
    TEST_ASSERT_EQUAL_INT(3, tmp_last->count);
    TEST_ASSERT_EQUAL_INT(964, tmp_last->hello);
}

void test_check_add_order_first(void) {
    int exp_ord[] = {1, 2, 0, 4, 3};
    test_t *node5 = (test_t *)malloc(sizeof(test_t));
    node5->data = 14;
    node5->count = 4;
    node5->hello = 529;
    doubly_linked_list_add_node_tail(__root, test_t, node5);
    test_t *tmp = NULL;
    test_t *tmp_last = NULL;
    int i = 0;
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
        tmp_last = tmp;
    }
    tmp = container_of(tmp_last->node.prev, test_t, node);
    TEST_ASSERT(tmp);
    TEST_ASSERT_EQUAL_INT(14, tmp->data);
    TEST_ASSERT_EQUAL_INT(4, tmp->count);
    TEST_ASSERT_EQUAL_INT(529, tmp->hello);
}

void test_check_foreach(void) {
    test_t *list[5] = {0};
    test_t *tmp = NULL;
    int i = 0;
    dl_for_each_node(tmp, __root, test_t) {
        list[i++] = tmp;
    }
    i = 0;
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT( list[i++] == tmp );
    }
    i = 0;
    dl_for_each_node_safe(tmp, __root, test_t) {
        TEST_ASSERT( list[i++] == tmp );
    }
}

static int testeq( test_t *s, int number ) {
    if ( s->count == number ) return 0;
    return -1;
}

void test_check_find_node(void) {
    test_t *tmp = NULL;
    doubly_linked_list_find_node(tmp, __root, test_t, testeq, 2);
    TEST_ASSERT( tmp );
    TEST_ASSERT_EQUAL_INT(12, tmp->data);
    TEST_ASSERT_EQUAL_INT(2, tmp->count);
    TEST_ASSERT_EQUAL_INT(528, tmp->hello);
}

void test_check_del_head(void) {
    int exp_ord[] = {1, 2, 0, 4};
    test_t *tmp = NULL;
    int i = 0;
    test_t *rm = __root;
    doubly_linked_list_del_node(__root, test_t, rm);
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
    }
    free(rm);
}

void test_check_del_last(void) {
    int exp_ord[] = {1, 2, 0};
    test_t *tmp = NULL;
    int i = 0;
    test_t *last;
    dl_for_each_node(tmp, __root, test_t) {
        last = tmp;
    }
    TEST_ASSERT( last );
    doubly_linked_list_del_node_head(__root, test_t);
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
    }
    free(last);
}

void test_check_del_middle(void) {
    int exp_ord[] = {1, 0};
    test_t *tmp = NULL;
    int i = 0;
    test_t *rm;
    doubly_linked_list_find_node(rm, __root, test_t, testeq, 2);
    TEST_ASSERT( rm );
    doubly_linked_list_del_node(__root, test_t, rm);
    dl_for_each_node(tmp, __root, test_t) {
        TEST_ASSERT_EQUAL_INT(exp_ord[i++], tmp->count);
    }
    free(rm);
}

void test_check_del_hard(void) {
    test_t *tmp = NULL;
    int rm_count = 0;
    dl_for_each_node_safe(tmp, __root, test_t) {
        free(tmp);
        rm_count ++ ;
    }
    TEST_ASSERT_EQUAL_INT( 2, rm_count );
}
