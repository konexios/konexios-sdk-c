#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <konexios_config.h>
#include <debug.h>
#include <arrow/credentials.h>
#include <data/static_buf.h>
#include <data/static_alloc.h>
#include <arrow/utf8.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/property_base.h>
#include <data/property_const.h>
#include <data/property_dynamic.h>
#include <data/property_stack.h>
#include <json/property_json.h>
#include <data/ringbuffer.h>
#include <data/propmap.h>
#include <json/json.h>
#include <json/sb.h>
#include <json/aob.h>
#include <encode.h>
#include <json/decode.h>
#include <bsd/socket.h>
#include <http/request.h>
#include <http/response.h>
#include <data/find_by.h>

#include <arrow/storage.h>
#include "fakestorage.h"
#include "storage_weak.h"

void setUp(void) {
    property_types_init();
}

void tearDown(void) {
    property_types_deinit();
}

static http_response_t _test_response;

extern int default_set_payload_handler(void *r,
                            property_t buf);

extern int default_add_payload_handler(void *r,
                            property_t payload);

void test_http_response_init(void) {
    _payload_meth_t def_handler = { default_set_payload_handler,
                                    default_add_payload_handler };
    http_response_init(&_test_response, &def_handler);
    TEST_ASSERT(_test_response._p_meth._p_add_handler);
    TEST_ASSERT(_test_response._p_meth._p_set_handler);
    TEST_ASSERT_EQUAL_INT(0, _test_response.is_chunked);
    TEST_ASSERT( IS_EMPTY(_test_response.payload) );
    TEST_ASSERT( ! _test_response.header );
    TEST_ASSERT( IS_EMPTY(_test_response.content_type.value) );
    TEST_ASSERT( IS_EMPTY(_test_response.content_type.key) );
    TEST_ASSERT( ! _test_response.content_type.node.next );
}

void test_http_request_add_header(void) {
    http_response_add_header(&_test_response, p_heap("hello"), p_heap("kokoko"));
    TEST_ASSERT( _test_response.header );
    property_map_t *tmp = NULL;
    arrow_linked_list_for_each(tmp, _test_response.header, property_map_t) {
        TEST_ASSERT_EQUAL_STRING("hello", P_VALUE(tmp->key));
        TEST_ASSERT_EQUAL_STRING("kokoko", P_VALUE(tmp->value));
    }
}

