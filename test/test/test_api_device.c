#include "unity.h"

#include <debug.h>
#include "api_gateway_gateway.h"
#include "api_gateway_info.h"
#include "http_routine.h"
#include "api_device_device.h"
#include "socket_weak.h"

#include <konexios/credentials.h>
#include <konexios/device.h>
#include <konexios/routine.h>
#include <konexios/transaction.h>
#include <konexios/api/device/event.h>
#include <konexios/api/device/info.h>
#include <konexios/api/json/parse.h>
#include <konexios/gateway.h>
#include <konexios/api/log.h>
#include <konexios/utf8.h>
#include <sys/mem.h>
#include <data/static_buf.h>
#include <data/static_alloc.h>
#include <konexios/sign.h>
#include <bsd/socket.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/property_base.h>
#include <data/property_const.h>
#include <data/property_dynamic.h>
#include <data/property_stack.h>
#include <json/property_json.h>
#include <data/ringbuffer.h>
#include <data/propmap.h>
#include <data/find_by.h>
#include <json/json.h>
#include <json/sb.h>
#include <json/aob.h>
#include <encode.h>
#include <json/decode.h>
#include <konexios_mqtt_client.h>
#include <mqtt/client/callback.h>
#include <http/client.h>
#include <http/request.h>
#include <http/response.h>
#include <http/routine.h>
#include <ssl/crypt.h>
#include <konexios/state.h>
#include <konexios/telemetry_api.h>
#include <konexios/mqtt.h>
#include <konexios/events.h>
#include <konexios/gateway_payload_sign.h>
#include <konexios/device_command.h>
#include <konexios/software_release.h>
#include <konexios/software_update.h>
#include <ssl/md5sum.h>
#include <http/client_mqtt.h>

#include <time/time.h>
#include "acnsdkc_time.h"
#include <network.h>

#include "timer.h"
#include "acnsdkc_ssl.h"
#include <MQTTClient.h>
#include <MQTTPacket.h>
#include <MQTTConnect.h>
#include <MQTTPublish.h>

#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/md5.h>

#include <acn.h>
#include "MQTTConnectClient.h"
#include "MQTTDeserializePublish.h"
#include "MQTTSerializePublish.h"
#include "MQTTSubscribeClient.h"
#include "MQTTUnsubscribeClient.h"

#include "mock_mac.h"
#include "mock_watchdog.h"
#include "mock_sockdecl.h"
#include "mock_telemetry.h"

#include "http_cb.h"
#include "fakedns.h"
#include "fakesock.h"
#include <konexios/storage.h>
#include "fakestorage.h"
#include "storage_weak.h"


#include "konexios_config.h"

#define GATEWAY_UID GATEWAY_UID_PREFIX "-111213141516"
#define TEST_GATEWAY_HID "000TEST000"
#define DEVICE_UID GATEWAY_UID "-" DEVICE_UID_SUFFIX

static konexios_gateway_t _test_gateway;
static konexios_gateway_config_t _test_gateway_config;
static konexios_device_t _test_device;

void setUp(void) {
    if ( konexios_init() < 0 ) {
        printf("konexios_init fail!\r\n");
        return;
    }
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    konexios_gateway_init(&_test_gateway);
    konexios_prepare_gateway(&_test_gateway);
    property_copy(&_test_gateway.hid, p_const(TEST_GATEWAY_HID));
}

void tearDown(void)
{
    konexios_deinit();
}

#define TEST_DEVICE_HID "d000000ca3a1a317222772437dc586cb59d680fe"

char http_resp_text[] =
        "HTTP/1.1 200 OK\r\n"
        "Date: Mon, 27 Jul 2018 12:28:53 GMT\r\n"
        "Pragma: no-cache\r\n"
        "x-content-type-options: nosniff\r\n"
        "x-frame-options: DENY\r\n"
        "Connection: keep-alive\r\n"
        "Content-type: application/json;charset=UTF-8\r\n"
        "Cache-control: no-cache, no-store, max-age=0, must-revalidate\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Strict-transport-security: max-age=31536000 ; includeSubDomains\r\n"
        "x-xss-protection: 1; mode=block\r\n"
        "Expires: 0\r\n"
        "\r\n"
        "4F\r\n"
        "{\"hid\":\""
        TEST_DEVICE_HID
        "\",\"links\": {}, \"message\": \"OK\"}\r\n00\r\n";

void test_konexios_register_device(void) {
    set_http_cb(http_resp_text, sizeof(http_resp_text));
    struct hostent *fake_addr = dns_fake(0xc0a80001, iotClientInitApi.host);
    struct sockaddr_in *serv = prepsock(fake_addr, iotClientInitApi.port);
    socket_ExpectAndReturn(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0);
    gethostbyname_ExpectAndReturn(iotClientInitApi.host, fake_addr);
    setsockopt_IgnoreAndReturn(0);
    connect_ExpectAndReturn(0, (struct sockaddr*)serv, sizeof(struct sockaddr_in), 0);
    send_StubWithCallback(send_cb);
    recv_StubWithCallback(recv_cb);
    soc_close_Expect(0);

    konexios_prepare_device(&_test_gateway, &_test_device);
    int ret = konexios_register_device(&_test_gateway, &_test_device);
    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_STRING(TEST_DEVICE_HID, P_VALUE(_test_device.hid));
}
