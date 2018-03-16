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

void test_http_response_init(void) {
    http_response_init(&_test_response, GET, "http://api.arrowconnect.io:80/api/v1/kronos/gateways");
    TEST_ASSERT(_test_request._response_payload_meth._p_add_handler);
    TEST_ASSERT(_test_request._response_payload_meth._p_set_handler);
    TEST_ASSERT_EQUAL_INT(0, _test_request.is_corrupt);
    TEST_ASSERT_EQUAL_STRING("api.arrowconnect.io", P_VALUE(_test_request.host));
    TEST_ASSERT_EQUAL_INT(80, _test_request.port);
    TEST_ASSERT_EQUAL_STRING("GET", P_VALUE(_test_request.meth));
    TEST_ASSERT_EQUAL_STRING("http", P_VALUE(_test_request.scheme));
    TEST_ASSERT( IS_EMPTY(_test_request.payload.buf) );
    TEST_ASSERT_EQUAL_INT(0, _test_request.payload.size);
    TEST_ASSERT( ! _test_request.header );
    TEST_ASSERT( IS_EMPTY(_test_request.content_type.value) );
    TEST_ASSERT( IS_EMPTY(_test_request.content_type.key) );
    TEST_ASSERT( ! _test_request.content_type.node.next );
}

void test_http_request_add_header(void) {
    http_request_add_header(&_test_request, p_heap("hello"), p_heap("kokoko"));
    TEST_ASSERT( _test_request.header );
    property_map_t *tmp = NULL;
    arrow_linked_list_for_each(tmp, _test_request.header, property_map_t) {
        TEST_ASSERT_EQUAL_STRING("hello", P_VALUE(tmp->key));
        TEST_ASSERT_EQUAL_STRING("kokoko", P_VALUE(tmp->value));
    }
}

