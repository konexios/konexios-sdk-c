#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <data/property.h>

void setUp(void)
{
}

void tearDown(void)
{
}

typedef struct test_prop {
    property_t name;
} test_p_t;

#define NAME "HELLO"

void test_property_init(void) {
    test_p_t test;
    P_CLEAR(test.name);
    TEST_ASSERT( !test.name.value );
    TEST_ASSERT( !test.name.flags );
}

void test_property_copy_const( void ) {
    test_p_t test;
    P_CLEAR(test.name);
    property_copy(&test.name, p_const(NAME));
    TEST_ASSERT( test.name.value == NAME );
    TEST_ASSERT_EQUAL_INT(is_const, test.name.flags);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}

void test_property_size( void ) {
    test_p_t test;
    P_CLEAR(test.name);
    TEST_ASSERT( IS_EMPTY(test.name) );
    property_copy(&test.name, p_const(NAME));
    TEST_ASSERT( !IS_EMPTY(test.name) );
    TEST_ASSERT_EQUAL_INT(5, P_SIZE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}


void test_property_copy_dynamic( void ) {
    test_p_t test;
    P_CLEAR(test.name);
    property_copy(&test.name, p_stack(NAME));
    TEST_ASSERT( test.name.value != NAME );
    TEST_ASSERT_EQUAL_INT(is_dynamic, test.name.flags);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}

void test_property_copy_malloc( void ) {
    test_p_t test;
    P_CLEAR(test.name);
    char *name = (char *)malloc(6);
    strcpy(name, NAME);
    property_copy(&test.name, p_heap(name));
    TEST_ASSERT( test.name.value == name );
    TEST_ASSERT_EQUAL_INT(is_dynamic, test.name.flags);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}

void test_property_n_copy( void ) {
    test_p_t test;
    P_CLEAR(test.name);
    property_n_copy(&test.name, NAME, 5);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}
