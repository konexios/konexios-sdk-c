#include "build/temp/_test_linkedlist.c"
#include "data/linkedlist.h"
#include "config.h"
#include "unity.h"


typedef struct _test_ {

  int data;

  int count;

  int hello;

  linked_list_t node;

} test_t;



void setUp(void)

{

}



void tearDown(void)

{

}



static test_t *__root = ((void *)0);



void test_create_linkedlist(void) {

    test_t *node = (test_t *)malloc(sizeof(test_t));

    node->data = 10;

    node->count = 0;

    node->hello = 111;

    { linked_list_t *base_p = linked_list_add((__root)?&(__root)->node:((void *)0), &(node)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    if ((__root)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(30)));};

    UnityAssertEqualNumber((UNITY_INT)((10)), (UNITY_INT)((__root->data)), (((void *)0)), (UNITY_UINT)(31), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((__root->count)), (((void *)0)), (UNITY_UINT)(32), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((111)), (UNITY_INT)((__root->hello)), (((void *)0)), (UNITY_UINT)(33), UNITY_DISPLAY_STYLE_INT);

    if ((!__root->node.next)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(34)));}

}



void test_add_create_linkedlist(void) {

    test_t *node2 = (test_t *)malloc(sizeof(test_t));

    node2->data = 11;

    node2->count = 1;

    node2->hello = 836;

    { linked_list_t *base_p = linked_list_add((__root)?&(__root)->node:((void *)0), &(node2)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    if ((__root->node.next)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(43)));};

    if ((!__root->node.next->next)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(44)));};



    test_t *node3 = (test_t *)malloc(sizeof(test_t));

    node3->data = 12;

    node3->count = 2;

    node3->hello = 528;

    { linked_list_t *base_p = linked_list_add((__root)?&(__root)->node:((void *)0), &(node3)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

}



void test_check_order(void) {

    int exp_ord[] = {0, 1, 2};

    test_t *tmp;

    int i = 0;

    linked_list_t *base_p57 = ((void *)0); for ( base_p57 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p57 != ((void *)0) ; base_p57 = base_p57->next, (tmp) = base_p57?((test_t *)((char *)(1 ? (base_p57) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(58), UNITY_DISPLAY_STYLE_INT);

    }

}



void test_check_add_order_last(void) {

    int exp_ord[] = {0, 1, 2, 3};

    test_t *node4 = (test_t *)malloc(sizeof(test_t));

    node4->data = 13;

    node4->count = 3;

    node4->hello = 964;

    { linked_list_t *base_p = linked_list_add((__root)?&(__root)->node:((void *)0), &(node4)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    test_t *tmp = ((void *)0);

    int i = 0;

    linked_list_t *base_p71 = ((void *)0); for ( base_p71 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p71 != ((void *)0) ; base_p71 = base_p71->next, (tmp) = base_p71?((test_t *)((char *)(1 ? (base_p71) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(72), UNITY_DISPLAY_STYLE_INT);

    }

    if ((tmp)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(74)));};

    UnityAssertEqualNumber((UNITY_INT)((13)), (UNITY_INT)((tmp->data)), (((void *)0)), (UNITY_UINT)(75), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((3)), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(76), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((964)), (UNITY_INT)((tmp->hello)), (((void *)0)), (UNITY_UINT)(77), UNITY_DISPLAY_STYLE_INT);

}



void test_check_add_order_first(void) {

    int exp_ord[] = {4, 0, 1, 2, 3};

    test_t *node5 = (test_t *)malloc(sizeof(test_t));

    node5->data = 14;

    node5->count = 4;

    node5->hello = 529;

    { linked_list_t *base_p = linked_list_add_first((__root)?&(__root)->node:((void *)0), &(node5)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    test_t *tmp = ((void *)0);

    int i = 0;

    linked_list_t *base_p89 = ((void *)0); for ( base_p89 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p89 != ((void *)0) ; base_p89 = base_p89->next, (tmp) = base_p89?((test_t *)((char *)(1 ? (base_p89) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(90), UNITY_DISPLAY_STYLE_INT);

    }

    if ((__root)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(92)));};

    UnityAssertEqualNumber((UNITY_INT)((14)), (UNITY_INT)((__root->data)), (((void *)0)), (UNITY_UINT)(93), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((4)), (UNITY_INT)((__root->count)), (((void *)0)), (UNITY_UINT)(94), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((529)), (UNITY_INT)((__root->hello)), (((void *)0)), (UNITY_UINT)(95), UNITY_DISPLAY_STYLE_INT);

}



void test_check_foreach(void) {

    test_t *list[5] = {0};

    test_t *tmp = ((void *)0);

    int i = 0;

    linked_list_t *base_p102 = ((void *)0); for ( base_p102 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p102 != ((void *)0) ; base_p102 = base_p102->next, (tmp) = base_p102?((test_t *)((char *)(1 ? (base_p102) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        list[i++] = tmp;

    }

    i = 0;

    linked_list_t *base_p106 = ((void *)0); for ( base_p106 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p106 != ((void *)0) ; base_p106 = base_p106->next, (tmp) = base_p106?((test_t *)((char *)(1 ? (base_p106) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        if ((list[i++] == tmp)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(107)));};

    }

    i = 0;

    linked_list_t *base_p110 = ((void *)0); linked_list_t base_p_obj110 = { ((void *)0) }; for ( base_p110 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root), base_p_obj110 = base_p110?*base_p110:base_p_obj110 ; base_p110 != ((void *)0) ; base_p110 = base_p_obj110.next, (tmp) = base_p110?((test_t *)((char *)(1 ? (base_p110) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):((void *)0), base_p_obj110 = (base_p110?(*base_p110):base_p_obj110) ) {

        if ((list[i++] == tmp)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(111)));};

    }

}



static int testeq( test_t *s, int number ) {

    if ( s->count == number ) return 0;

    return -1;

}



void test_check_find_node(void) {

    test_t *tmp = ((void *)0);

    { linked_list_t *base_p122 = ((void *)0); for ( base_p122 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p122 != ((void *)0) ; base_p122 = base_p122->next, (tmp) = base_p122?((test_t *)((char *)(1 ? (base_p122) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) { if ( (testeq)((tmp), (2)) == 0 ) break; (tmp) = ((void *)0); } };

    if ((tmp)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(123)));};

    UnityAssertEqualNumber((UNITY_INT)((12)), (UNITY_INT)((tmp->data)), (((void *)0)), (UNITY_UINT)(124), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((2)), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(125), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((528)), (UNITY_INT)((tmp->hello)), (((void *)0)), (UNITY_UINT)(126), UNITY_DISPLAY_STYLE_INT);

}



void test_check_del_first(void) {

    int exp_ord[] = {0, 1, 2, 3};

    test_t *tmp = ((void *)0);

    int i = 0;

    test_t *rm = __root;

    { linked_list_t *base_p = linked_list_del(&(__root)->node, &(__root)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    linked_list_t *base_p135 = ((void *)0); for ( base_p135 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p135 != ((void *)0) ; base_p135 = base_p135->next, (tmp) = base_p135?((test_t *)((char *)(1 ? (base_p135) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(136), UNITY_DISPLAY_STYLE_INT);

    }

    free(rm);

}



void test_check_del_last(void) {

    int exp_ord[] = {0, 1, 2};

    test_t *tmp = ((void *)0);

    int i = 0;

    test_t *last;

    linked_list_t *base_p146 = ((void *)0); for ( base_p146 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p146 != ((void *)0) ; base_p146 = base_p146->next, (tmp) = base_p146?((test_t *)((char *)(1 ? (base_p146) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        ;

    }

    last = tmp;

    if ((last)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(150)));};

    { linked_list_t *base_p = linked_list_del_last(&(__root)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    linked_list_t *base_p152 = ((void *)0); for ( base_p152 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p152 != ((void *)0) ; base_p152 = base_p152->next, (tmp) = base_p152?((test_t *)((char *)(1 ? (base_p152) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(153), UNITY_DISPLAY_STYLE_INT);

    }

    free(last);

}



void test_check_del_middle(void) {

    int exp_ord[] = {0, 2};

    test_t *tmp = ((void *)0);

    int i = 0;

    test_t *rm;

    { linked_list_t *base_p163 = ((void *)0); for ( base_p163 = (__root)?&(__root)->node:((void *)0), (rm) = (__root) ; base_p163 != ((void *)0) ; base_p163 = base_p163->next, (rm) = base_p163?((test_t *)((char *)(1 ? (base_p163) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(rm) ) { if ( (testeq)((rm), (1)) == 0 ) break; (rm) = ((void *)0); } };

    if ((rm)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(164)));};

    { linked_list_t *base_p = linked_list_del(&(__root)->node, &(rm)->node); __root = ((test_t *)((char *)(1 ? (base_p) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))); };

    linked_list_t *base_p166 = ((void *)0); for ( base_p166 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p166 != ((void *)0) ; base_p166 = base_p166->next, (tmp) = base_p166?((test_t *)((char *)(1 ? (base_p166) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        UnityAssertEqualNumber((UNITY_INT)((exp_ord[i++])), (UNITY_INT)((tmp->count)), (((void *)0)), (UNITY_UINT)(167), UNITY_DISPLAY_STYLE_INT);

    }

    free(rm);

}



void test_check_del_hard(void) {

    test_t *tmp;

    linked_list_t *base_p174 = ((void *)0); for ( base_p174 = (__root)?&(__root)->node:((void *)0), (tmp) = (__root) ; base_p174 != ((void *)0) ; base_p174 = base_p174->next, (tmp) = base_p174?((test_t *)((char *)(1 ? (base_p174) : &((test_t *)0)->node) - __builtin_offsetof (test_t, node))):(tmp) ) {

        free(tmp);

        tmp = ((void *)0);

    }

    if ((!tmp)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(178)));};

}
