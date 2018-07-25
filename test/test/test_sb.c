#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <debug.h>
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

static uint8_t buffer[100];

void setUp(void) {
    property_types_init();
    property_type_add(property_type_get_json());
    property_type_add(property_type_get_aob());
    memset(buffer, 0x0, sizeof(buffer));
}

void tearDown(void) {
    property_types_deinit();
}

void test_alloc_only_init(void) {
    alloc_only_t mem;
    property_t tmp = { buffer, PROPERTY_CONST_TAG | is_owner, sizeof(buffer) };
    alloc_only_memory_set(&mem, &tmp);
    TEST_ASSERT_EQUAL_PTR(buffer, mem.start);
    TEST_ASSERT_EQUAL_INT(0, mem.len);
    TEST_ASSERT_EQUAL_INT(sizeof(buffer), mem.size);
}

void test_alloc_only_put_char( void ) {
    alloc_only_t mem;
    property_t tmp = { buffer, PROPERTY_CONST_TAG | is_owner, sizeof(buffer) };
    alloc_only_memory_set(&mem, &tmp);
    alloc_only_init(&mem);
    alloc_only_put(&mem, 'c');
    TEST_ASSERT_EQUAL_INT('c', (buffer[0]));
    TEST_ASSERT_EQUAL_INT(1, mem.len);
    TEST_ASSERT_EQUAL_INT(sizeof(buffer), mem.size);
}

void test_alloc_only_puts_char( void ) {
    alloc_only_t mem;
    property_t tmp = { buffer, PROPERTY_CONST_TAG | is_owner, sizeof(buffer) };
    alloc_only_memory_set(&mem, &tmp);
    alloc_only_init(&mem);
    alloc_only_puts(&mem, "hello", 5);
    TEST_ASSERT_EQUAL_STRING("hello", mem.start);
    TEST_ASSERT_EQUAL_INT(5, mem.len);
    TEST_ASSERT_EQUAL_INT(sizeof(buffer), mem.size);
    char zeros[95] = { 0 };
    TEST_ASSERT_EQUAL_MEMORY( zeros, buffer+5, 95);
}

#define first_test_word "first"
#define second_test_word "second"
void test_alloc_only_puts_2char( void ) {
    alloc_only_t mem;
    property_t tmp = { buffer, PROPERTY_CONST_TAG | is_owner, sizeof(buffer) };
    alloc_only_memory_set(&mem, &tmp);
    alloc_only_init(&mem);
    alloc_only_puts(&mem, first_test_word, strlen(first_test_word));
    char *first = alloc_only_finish(&mem);
    alloc_only_puts(&mem, second_test_word, strlen(second_test_word));
    char *second = alloc_only_finish(&mem);
    TEST_ASSERT_EQUAL_STRING(first_test_word, first);
    TEST_ASSERT_EQUAL_STRING(second_test_word, second);
    int total_len = strlen(first_test_word) + strlen(second_test_word) + 2;
    TEST_ASSERT_EQUAL_INT(sizeof(buffer)-total_len, mem.size);
    char zeros[sizeof(buffer) - sizeof(first_test_word)-sizeof(second_test_word)+4] = { 0 };
    TEST_ASSERT_EQUAL_MEMORY( zeros, buffer+total_len, sizeof(buffer) - total_len);
}


