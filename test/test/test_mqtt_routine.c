#include "unity.h"
#include <config.h>

#include <debug.h>
#include "api_gateway_info.h"
#include "http_routine.h"
#include "socket_weak.h"

#include <arrow/credentials.h>
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
#include <mqtt/client/callback.h>
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
#include "mock_telemetry.h"
#include "mock_api_gateway_gateway.h"
#include "mock_api_device_device.h"

#include <arrow/storage.h>
#include "http_cb.h"
#include "fakedns.h"
#include "fakesock.h"
#include "fakestorage.h"
#include "storage_weak.h"

void setUp(void) {
    if ( arrow_init() < 0 ) {
        printf("arrow_init fail!\r\n");
        return;
    }
}

void tearDown(void) {
    arrow_deinit();
}

#define TEST_GATEWAY_HID "e000000f63b1a317222772437dc586cb59d680fe"
#define TEST_DEVICE_HID "86241a5e939baa7da58faff3527eb021d06d5e9a"

static int test_cmd_proc(property_t p) {
  printf("test: [%s]\t\n", P_VALUE(p));
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
char mqtt_pub_text[] = { "\x32\x89\x02\x00\x34"
                         "krs/cmd/stg/e000000f63b1a317222772437dc586cb59d680fe"
                         "\x00\x01"
                         "{\"hid\":\"c70d5081431066c797cabe7067ad714a7d812770\","
                          "\"name\":\"ServerToGateway_DeviceCommand\","
                          "\"encrypted\":false,"
                          "\"parameters\":{"
                           "\"deviceHid\":\"86241a5e939baa7da58faff3527eb021d06d5e9a\","
                           "\"command\":\"test\","
                           "\"payload\":\"{}\""
                          "}"
                         "}"
                         };
char mqtt_puback_text[] = { 0x40, 0x02, 0x00, 0x01 };

char mqtt_api_pub_text[] = { "\x32\xb9\x02\x00\x34"
                             "krs/api/stg/e000000f63b1a317222772437dc586cb59d680fe"
                             "\x00\x01"
                             "{\"requestId\":\"XYZ-123\","
                              "\"eventName\":\"GatewayToServer_ApiRequest\","
                              "\"encrypted\":false,"
                              "\"parameters\": {"
                                     "\"status\": \"OK\","
                                     "\"message\": \"Custom message is optional\""
                                 "},"
                                 "\"signature\": \"fa0bbd16481aeef45d320a941bcc6cb390b4d028f15c918465ce5ecc45359482\","
                                 "\"signatureVersion\": \"1\""
                             "}"
                         };

#if defined(HTTP_VIA_MQTT)
void test_mqtt_connect(void) {
    char mac[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    struct hostent *fake_addr = dns_fake(0xc0a80001, MQTT_COMMAND_ADDR);
    struct sockaddr_in *serv = prepsock(fake_addr, MQTT_PORT);

    wdt_feed_IgnoreAndReturn(0);
    fake_set_gateway_hid(TEST_GATEWAY_HID);
    fake_set_device_hid(TEST_DEVICE_HID);
    arrow_gateway_checkin_IgnoreAndReturn(0);
    arrow_gateway_update_IgnoreAndReturn(0);
    get_mac_address_ExpectAnyArgsAndReturn(0);
    get_mac_address_ReturnArrayThruPtr_mac(mac, 6);
    arrow_register_gateway_IgnoreAndReturn(0);
    set_http_cb(mqtt_connack_text,
                sizeof(mqtt_connack_text));
    add_http_cb(NULL, -1);
    add_http_cb(mqtt_suback_text, sizeof(mqtt_suback_text));
    add_http_cb(mqtt_suback_text, sizeof(mqtt_suback_text));
    printf("[%s] {%d}\r\n", mqtt_pub_text+59, sizeof(mqtt_pub_text)-1);
    printf("[%s] {%d}\r\n", mqtt_api_pub_text+59, sizeof(mqtt_api_pub_text)-1);
    add_http_cb(mqtt_pub_text, sizeof(mqtt_pub_text)-1);
    // receive
    add_http_cb(mqtt_puback_text, sizeof(mqtt_puback_text));
    add_http_cb(mqtt_api_pub_text, sizeof(mqtt_api_pub_text)-1);
    // fin
    add_http_cb(mqtt_puback_text, sizeof(mqtt_puback_text));
    add_http_cb(mqtt_api_pub_text, sizeof(mqtt_api_pub_text)-1);

    arrow_gateway_config_IgnoreAndReturn(0);

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
    property_copy(&current_device()->hid, p_const(TEST_DEVICE_HID));
    property_copy(&current_device()->gateway_hid, p_const(TEST_GATEWAY_HID));

    ret = arrow_mqtt_connect_routine();
    TEST_ASSERT_EQUAL_INT(ROUTINE_SUCCESS, ret);

    ret = arrow_mqtt_event_receive_routine();
    TEST_ASSERT_EQUAL_INT(ROUTINE_RECEIVE_EVENT, ret);

    TEST_ASSERT(arrow_mqtt_has_events());
    arrow_mqtt_event_proc();

    TEST_ASSERT_EQUAL_INT(ARROW_JSON_STATIC_BUFFER_SIZE, json_static_memory_max_sector());
}
#endif
