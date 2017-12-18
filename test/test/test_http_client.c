#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
//#include <http/client.h>
//#include <sys/mem.h>
//#include <data/property.h>
//#include <data/ringbuffer.h>
//#include <ssl/ssl.h>
#include <json/json.h>
#include "mock_mac.h"

void setUp(void)
{
}

void tearDown(void)
{
}

//static http_client_t _test_cli = { -1, -1, 0, {1, 1}, NULL, NULL, NULL };

void test_client_init(void) {
    /*
    http_client_init(&_test_cli);
    TEST_ASSERT_EQUAL_INT(0, _test_cli.response_code);
    TEST_ASSERT( _test_cli.queue );
    TEST_ASSERT_EQUAL_INT(-1, _test_cli.sock);
    TEST_ASSERT_EQUAL_INT(DEFAULT_API_TIMEOUT, _test_cli.timeout);
    */
}
/*
void test_gateway_prepare( void ) {
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
//    get_mac_address_ExpectAndReturn(mac, 0);
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    arrow_prepare_gateway(&_test_gateway);
}

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
