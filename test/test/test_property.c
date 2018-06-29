#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <data/static_buf.h>
#include <data/static_alloc.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/property_base.h>
#include <data/property_const.h>
#include <data/property_dynamic.h>
#include <data/property_stack.h>
#include <json/property_json.h>
#include <json/json.h>
#include <json/sb.h>
#include <json/aob.h>
#include <encode.h>
#include <json/decode.h>

void setUp(void) {
    property_types_init();
    property_type_add(property_type_get_json());
}

void tearDown(void) {
    property_types_deinit();
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
    TEST_ASSERT_EQUAL_INT(PROPERTY_CONST_TAG, PROPERTY_BASE_MASK & test.name.flags);
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
    TEST_ASSERT_EQUAL_INT(PROPERTY_DYNAMIC_TAG, PROPERTY_BASE_MASK & test.name.flags);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}

void test_property_move_dynamic( void ) {
    test_p_t test;
    property_init(&test.name);
    property_t p = string_to_dynamic_property(NAME);
    property_move(&test.name, &p);
    TEST_ASSERT_EQUAL_STRING ( NAME, test.name.value );
    TEST_ASSERT_EQUAL_INT(PROPERTY_DYNAMIC_TAG, PROPERTY_BASE_MASK & test.name.flags);
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}

void test_property_json_copy( void ) {
    test_p_t test;
    property_init(&test.name);
    property_copy(&test.name, json_strdup_property(NAME));
    TEST_ASSERT_EQUAL_STRING(NAME, P_VALUE(test.name));
    property_free(&test.name);
    TEST_ASSERT( !test.name.value );
}
