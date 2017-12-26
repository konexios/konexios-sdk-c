#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <http/client.h>
#include <sys/mem.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/ringbuffer.h>
#include <data/propmap.h>
#include <json/json.h>
#include <bsd/socket.h>
#include <http/request.h>
#include <http/response.h>
#include <data/find_by.h>
#include "mock_mac.h"
#include "mock_sockdecl.h"
#include "mock_ssl.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static http_client_t _test_cli = { -1, -1, 0, {1, 1}, NULL, NULL, NULL };

void test_client_init(void) {
    http_client_init(&_test_cli);
    TEST_ASSERT_EQUAL_INT(0, _test_cli.response_code);
    TEST_ASSERT( _test_cli.queue );
    TEST_ASSERT_EQUAL_INT(-1, _test_cli.sock);
    TEST_ASSERT_EQUAL_INT(DEFAULT_API_TIMEOUT, _test_cli.timeout);
}

void test_client_do( void ) {
    http_request_t request;
    http_response_t response;

    static struct hostent s_hostent;
    static char *s_aliases;
    static unsigned long s_hostent_addr;
    static unsigned long *s_phostent_addr[2];

    s_hostent_addr = 0xc0a80001;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = NULL;
    s_hostent.h_name = (char*) ARROW_ADDR;
    s_hostent.h_aliases = &s_aliases;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(unsigned long);
    s_hostent.h_addr_list = (char**) &s_phostent_addr;
    s_hostent.h_addr = s_hostent.h_addr_list[0];

//    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
//    get_mac_address_ExpectAndReturn(mac, 0);
    socket_ExpectAndReturn(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0);
    gethostbyname_ExpectAndReturn(0, &s_hostent);
//    gethostbyname_IgnoreAndReturn(&s_hostent);
//    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    int ret = http_client_do(&_test_cli, &request, &response);
    TEST_ASSERT_EQUAL_INT(0, ret);
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
