#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <arrow/gateway.h>
#include <arrow/mem.h>
#include <json/json.h>
#include "mock_net.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static arrow_gateway_t _test_gateway;

void test_gateway_init(void) {
    arrow_gateway_init(&_test_gateway);
    TEST_ASSERT_EQUAL_INT(0, P_VALUE(_test_gateway.hid));
}

void test_gateway_prepare( void ) {
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
//    get_mac_address_ExpectAndReturn(mac, 0);
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    arrow_prepare_gateway(&_test_gateway);
    printf("_test_gateway %s", P_VALUE(_test_gateway.uid));
}
