#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/ringbuffer.h>
#include <data/propmap.h>
#include <json/json.h>
#include <bsd/socket.h>
#include <http/request.h>
#include <http/response.h>
#include <data/find_by.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static http_response_t _test_response;

extern int default_set_payload_handler(void *r,
                            property_t buf,
                            int size);

extern int default_add_payload_handler(void *r,
                            property_t payload,
                            int size);

void test_http_response_init(void) {
    _payload_meth_t def_handler = { default_set_payload_handler,
                                    default_add_payload_handler };
    http_response_init(&_test_response, &def_handler);
    TEST_ASSERT(_test_response._p_meth._p_add_handler);
    TEST_ASSERT(_test_response._p_meth._p_set_handler);
    TEST_ASSERT_EQUAL_INT(0, _test_response.is_chunked);
    TEST_ASSERT( IS_EMPTY(_test_response.payload.buf) );
    TEST_ASSERT_EQUAL_INT(0, _test_response.payload.size);
    TEST_ASSERT( ! _test_response.header );
    TEST_ASSERT( IS_EMPTY(_test_response.content_type.value) );
    TEST_ASSERT( IS_EMPTY(_test_response.content_type.key) );
    TEST_ASSERT( ! _test_response.content_type.node.next );
}

void test_http_request_add_header(void) {
    http_response_add_header(&_test_response, p_heap("hello"), p_heap("kokoko"));
    TEST_ASSERT( _test_response.header );
    property_map_t *tmp = NULL;
    for_each_node(tmp, _test_response.header, property_map_t) {
        TEST_ASSERT_EQUAL_STRING("hello", P_VALUE(tmp->key));
        TEST_ASSERT_EQUAL_STRING("kokoko", P_VALUE(tmp->value));
    }
}

