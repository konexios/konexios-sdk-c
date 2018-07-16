#include "unity.h"
#include <config.h>

#include "api_gateway_info.h"
#include "http_routine.h"
#include "socket_weak.h"

#include <arrow/api/device/event.h>
#include <arrow/device.h>
#include <arrow/api/device/info.h>
#include <arrow/api/json/parse.h>
#include <arrow/gateway.h>
#include <arrow/api/log.h>
#include <arrow/utf8.h>
#include <sys/mem.h>
#include <data/static_buf.h>
#include <data/static_alloc.h>
#include <arrow/sign.h>
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
#include <arrow_mqtt_client.h>
#include <mqtt/client/delivery.h>
#include <http/client.h>
#include <http/request.h>
#include <http/response.h>
#include <ssl/crypt.h>
#include <arrow/state.h>
#include <arrow/routine.h>
#include <arrow/telemetry_api.h>
#include <arrow/mqtt.h>
#include <arrow/events.h>
#include <arrow/gateway_payload_sign.h>
#include <arrow/device_command.h>
#include <arrow/software_release.h>
#include <arrow/software_update.h>
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
#include "mock_storage.h"
#include "mock_telemetry.h"
#include "mock_api_gateway_gateway.h"
#include "mock_api_device_device.h"

#include "http_cb.h"
#include "fakedns.h"
#include "fakesock.h"

void setUp(void) {
    arrow_init();
}

void tearDown(void) {
    arrow_deinit();
}

#define TEST_GATEWAY_HID "e000000f63b1a317222772437dc586cb59d680fe"

static int test_cmd_proc(const char *s) {
  printf("test: [%s]\t\n", s);
//  printf("static json buffer before %d\r\n", json_static_memory_max_sector());
//  JsonNode *j = json_decode(str);
//  if ( !j ) return -1;
//  show_json_obj(j);
//  printf("static json buffer after %d\r\n", json_static_memory_max_sector());
//  json_delete(j);
  return 0;
}

char mqtt_connack_text[] = { 0x20, 0x02, 0x01, 0x00 };
char mqtt_suback_text[] = { 0x90, 0x03, 0x00, 0x02, 0x01 };

void test_mqtt_connect(void) {
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    struct hostent *fake_addr = dns_fake(0xc0a80001, MQTT_COMMAND_ADDR);
    struct sockaddr_in *serv = prepsock(fake_addr, MQTT_PORT);

    wdt_feed_IgnoreAndReturn(0);
    restore_gateway_info_IgnoreAndReturn(0);
    arrow_gateway_checkin_IgnoreAndReturn(0);
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    arrow_register_gateway_IgnoreAndReturn(0);
    set_http_cb(mqtt_connack_text,
                sizeof(mqtt_connack_text));
    add_http_cb(NULL, -1);
    add_http_cb(mqtt_suback_text, sizeof(mqtt_suback_text));
    add_http_cb(mqtt_suback_text, sizeof(mqtt_suback_text));


    save_gateway_info_Ignore();
    arrow_gateway_config_IgnoreAndReturn(0);
    restore_device_info_IgnoreAndReturn(0);

    gethostbyname_ExpectAndReturn(MQTT_COMMAND_ADDR, fake_addr);
    socket_ExpectAndReturn(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0);
    connect_ExpectAndReturn(0, (struct sockaddr*)serv, sizeof(struct sockaddr_in), 0);
    setsockopt_IgnoreAndReturn(0);
    send_StubWithCallback(send_cb);
    recv_StubWithCallback(recv_cb);
//    soc_close_Expect(0);

    arrow_command_handler_add("test", &test_cmd_proc);
    arrow_routine_error_t ret = arrow_initialize_routine();
    TEST_ASSERT_EQUAL_INT(ROUTINE_SUCCESS, ret);

    property_copy(&current_gateway()->hid, p_const(TEST_GATEWAY_HID));

    ret = arrow_mqtt_connect_routine();
    TEST_ASSERT_EQUAL_INT(ROUTINE_SUCCESS, ret);
//    TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_HID, P_VALUE(_test_gateway.hid));
}

#if 0
#define TEST_API_KEY "c4fc3a08c2015b8571dc2e3fce499aac96d312c200f86b2d37359c2e1dfecd9a"
#define TEST_SECRET_KEY "h/uV+qjul81kSb9564yBbCo9lAkTchOsMlqtlS1htUg="
char gateway_config_text[] =
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
        "B4\r\n"
        "{ \"cloudPlatform\": \"IotConnect\","
        "\"key\": \{"
        "\"apiKey\": \""
        TEST_API_KEY
        "\","
        "\"secretKey\": \""
        TEST_SECRET_KEY
        "\" }}"
        "\r\n00\r\n";

void test_arrow_gateway_initialize_routine() {
    set_http_cb(gateway_register_text, sizeof(gateway_register_text));
    add_http_cb(gateway_config_text, sizeof(gateway_config_text));
    struct hostent *fake_addr = dns_fake(0xc0a80001, ARROW_ADDR);
    struct sockaddr_in *serv = prepsock(fake_addr, ARROW_PORT);
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    wdt_feed_IgnoreAndReturn(0);
    restore_gateway_info_IgnoreAndReturn(-1);
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    socket_ExpectAndReturn(PF_INET, SOCK_STREAM, IPPROTO_TCP, 0);
    gethostbyname_ExpectAndReturn(ARROW_ADDR, fake_addr);
    setsockopt_IgnoreAndReturn(0);
    connect_ExpectAndReturn(0, (struct sockaddr*)serv, sizeof(struct sockaddr_in), 0);
    send_StubWithCallback(send_cb);
    recv_StubWithCallback(recv_cb);
    save_gateway_info_Expect(current_gateway());
    soc_close_Expect(0);

    int ret = arrow_gateway_initialize_routine();
    TEST_ASSERT_EQUAL_INT(ROUTINE_SUCCESS, ret);
    TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_HID, P_VALUE(current_gateway()->hid));
    TEST_ASSERT_EQUAL_INT(IoT, current_gateway_config()->type);
    TEST_ASSERT_EQUAL_STRING(TEST_API_KEY, get_api_key());
    TEST_ASSERT_EQUAL_STRING(TEST_SECRET_KEY, get_secret_key());
}
#endif
