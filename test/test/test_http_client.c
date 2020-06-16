#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <konexios_config.h>
#include <debug.h>
#include <arrow/credentials.h>
#include <http/client.h>
#include <sys/mem.h>
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

#include "acnsdkc_ssl.h"

#include "mock_mac.h"
#include "mock_sockdecl.h"
#include "socket_weak.h"

#include "http_cb.h"
#include "fakedns.h"
#include "fakesock.h"
#include <arrow/storage.h>
#include "storage_weak.h"

void setUp(void) {
    property_types_init();
    arrow_hosts_init();
}

void tearDown(void) {
    property_types_deinit();
}

static http_client_t _test_cli;

void test_client_init(void) {
    http_client_init(&_test_cli);
    TEST_ASSERT_EQUAL_INT(0, _test_cli.response_code);
    TEST_ASSERT( _test_cli.queue );
    TEST_ASSERT_EQUAL_INT(-1, _test_cli.sock);
    TEST_ASSERT_EQUAL_INT(DEFAULT_API_TIMEOUT, _test_cli.timeout);
}

char http_resp_text[] =
        "HTTP/1.1 200 OK\r\n"
        "Date: Mon, 27 Jul 2018 12:28:53 GMT\r\n"
        "\r\n";

static http_request_t request;
static http_response_t response;

void test_http_client_do( void ) {
    set_http_cb(http_resp_text, sizeof(http_resp_text));

   struct hostent *fake_addr = dns_fake(0xc0a80001, iotClientInitApi.host);

    // test address
    http_request_init(&request, GET, &p_const("/api/v1/kronos/gateways"));
    TEST_ASSERT( !request.query );

    socket_ExpectAndReturn(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0);
    gethostbyname_ExpectAndReturn(P_VALUE(request.host), fake_addr);
    struct sockaddr_in *serv = prepsock(fake_addr, request.port);

    setsockopt_IgnoreAndReturn(0);
    connect_ExpectAndReturn(0, (struct sockaddr*)serv, sizeof(struct sockaddr_in), 0);

    send_StubWithCallback(send_cb);
    recv_StubWithCallback(recv_cb);

    http_client_open(&_test_cli, &request);

    int ret = http_client_do(&_test_cli, &response);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_INT(200, response.m_httpResponseCode);
}

void test_http_client_free( void ) {
    soc_close_Expect(0);
    http_client_close(&_test_cli);
    http_client_free(&_test_cli);
    TEST_ASSERT(!_test_cli.queue);
}

/*
void test_gateway_serialize( void ) {
    char *t = arrow_gateway_serialize(&_test_gateway);
    const char *test = "{\"name\":\"probook-gateway-demo\","
                      "\"uid\":\"probook-111213141516\","
                      "\"osName\":\"linux\","
                      "\"type\":\"" GATEWAY_TYPE "\","
                      "\"softwareName\":\"" GATEWAY_SOFTWARE_NAME "\","
                      "\"softwareVersion\":\"" GATEWAY_SOFTWARE_VERSION "\","
                      "\"sdkVersion\":\"" xstr(SDK_VERSION) "\"}";
    TEST_ASSERT_EQUAL_STRING(test, t);
    free(t);
}
#define TEST_HID "9be7ab0b255cecb726cc912ddc1f29e57a9cbdd3"
void test_gateway_response( void ) {
    char *serv_resp = "{ \"hid\":\"" TEST_HID "\" }";
    arrow_gateway_parse(&_test_gateway, serv_resp);
    TEST_ASSERT_EQUAL_STRING(TEST_HID, P_VALUE(_test_gateway.hid));
}
*/
