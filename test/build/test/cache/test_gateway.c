#include "build/temp/_test_gateway.c"
#include "mock_net.h"
#include "json/json.h"
#include "data/property.h"
#include "arrow/mem.h"
#include "arrow/gateway.h"
#include "config.h"
#include "unity.h"








void setUp(void)

{

}



void tearDown(void)

{

}



static arrow_gateway_t _test_gateway;



void test_gateway_init(void) {

    arrow_gateway_init(&_test_gateway);

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((( (_test_gateway.hid).value ))), (((void *)0)), (UNITY_UINT)(23), UNITY_DISPLAY_STYLE_INT);

}



void test_gateway_prepare( void ) {

    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};



    get_mac_address_CMockExpectAnyArgsAndReturn(29, 0);

    get_mac_address_CMockReturnMemThruPtr_mac(30, mac, (int)(6 * (int)sizeof(*mac)));

    arrow_prepare_gateway(&_test_gateway);

    printf("_test_gateway %s", ( (_test_gateway.uid).value ));

}
