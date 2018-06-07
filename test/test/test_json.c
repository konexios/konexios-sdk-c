#include "unity.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/property_base.h>
#include <data/property_const.h>
#include <data/property_dynamic.h>
#include <data/property_stack.h>
#include <json/property_json.h>
#include <json/json.h>

void setUp(void) {
    property_types_init();
    property_type_add(property_type_get_json());
}

void tearDown(void) {
    property_types_deinit();
}

typedef struct test_prop {
    property_t name;
} test_p_t;

#define NAME "HELLO"

static char test[512] = {0};
#define JSON_INT_EX "{\"%s\":%d}"
#define JSON_BOOL_EX "{\"%s\":%s}"
#define JSON_STR_EX "{\"%s\":\"%s\"}"

#define bool2str(x) (x?"true":"false")

void test_parse_json_number(void) {
    snprintf(test, sizeof(test), JSON_INT_EX, "key", 100);
    JsonNode *_main = json_decode(test);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_NUMBER, _key->tag );
    TEST_ASSERT_EQUAL_INT( 100, _key->number_ );
}

void test_parse_json_bool_true(void) {
    snprintf(test, sizeof(test), JSON_BOOL_EX, "key", bool2str(true));
    JsonNode *_main = json_decode(test);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_BOOL, _key->tag );
    TEST_ASSERT( _key->bool_ );
}

void test_parse_json_bool_false(void) {
    snprintf(test, sizeof(test), JSON_BOOL_EX, "key", bool2str(false));
    JsonNode *_main = json_decode(test);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_BOOL, _key->tag );
    TEST_ASSERT( !_key->bool_ );
}

void test_parse_json_string(void) {
    snprintf(test, sizeof(test), JSON_STR_EX, "key", "hello");
    JsonNode *_main = json_decode(test);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_STRING, _key->tag );
    TEST_ASSERT_EQUAL_INT( 0, strcmp(_key->string_, "hello") );
}

#define JSON_OBJ_EX "{\"key\":{\"child\":\"yes\"}}"
void test_parse_json_obj(void) {
    JsonNode *_main = json_decode(JSON_OBJ_EX);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_OBJECT, _key->tag );

    JsonNode *_child = json_find_member(_key, p_const("child"));
    TEST_ASSERT_EQUAL_INT( 0, strcmp(_child->string_, "yes") );
}

#define JSON_ARRAY_EX "{\"key\":[{\"child\":1}, {\"child\":2}, {\"child\":3}]}"
void test_parse_json_array(void) {
    JsonNode *_main = json_decode(JSON_ARRAY_EX);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( JSON_ARRAY, _key->tag );

    JsonNode *_tmp = NULL;
    int num = 1;
    json_foreach(_tmp, _key) {
        JsonNode *_child = json_find_member(_tmp, p_const("child"));
        TEST_ASSERT(_child);
        TEST_ASSERT_EQUAL_INT( num, _child->number_ );
        num++;
    }
    TEST_ASSERT_EQUAL_INT( 4, num );
}

char complex_json_text[] =
        "{\"cloudPlatform\":\"IotConnect\","
        "\"key\":{\"apiKey\":\"dcbd9aa24b1d86524fc54d9f721f186f70f8a3158899f2243393658bff1d4a60\","
        "\"secretKey\":\"B41+VQRV/VcC4lnaxaFcd3rHffVZ6WdbqdxSPFP2qr8=\"}}"
        ;
void test_parse_json_complex(void) {
    JsonNode *_main = json_decode(complex_json_text);
    TEST_ASSERT(_main);
    JsonNode *_cloud = json_find_member(_main, p_const("cloudPlatform"));
    TEST_ASSERT(_cloud);
    TEST_ASSERT_EQUAL_STRING("IotConnect", _cloud->string_);
    JsonNode *_key = json_find_member(_main, p_const("key"));
    TEST_ASSERT(_key);
    JsonNode *_api = json_find_member(_key, p_const("apiKey"));
    TEST_ASSERT(_api);
    TEST_ASSERT_EQUAL_STRING("dcbd9aa24b1d86524fc54d9f721f186f70f8a3158899f2243393658bff1d4a60", _api->string_);
    JsonNode *_secret = json_find_member(_key, p_const("secretKey"));
    TEST_ASSERT(_secret);
    TEST_ASSERT_EQUAL_STRING("B41+VQRV/VcC4lnaxaFcd3rHffVZ6WdbqdxSPFP2qr8=", _secret->string_);
}

char complex2_json_text[] =
        "[ {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_zbxM2a_11:46 AM\","
        "\"name\" : \"FEED 2\","
        "\"portion\" : 0.0625,"
        "\"reminder\" : false,"
        "\"time\" : 42360"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_1UKZ8f_3:46 PM\","
        "\"name\" : \"FEED 3\","
        "\"portion\" : 0.0625,"
        "\"reminder\" : false,"
        "\"time\" : 56760"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_YMbDPV_4:28 PM\","
        "\"name\" : \"FEED 2\","
        "\"portion\" : 0.0625,"
        "\"reminder\" : false,"
        "\"time\" : 59280"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_p8HCLb_5:10 PM\","
        "\"name\" : \"FEED 4\","
        "\"portion\" : 0.0625,"
        "\"reminder\" : false,"
        "\"time\" : 61800"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_ognZKK_8:15 PM\","
        "\"name\" : \"FEED 7\","
        "\"portion\" : 0.375,"
        "\"reminder\" : false,"
        "\"time\" : 72900"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_Fpz5qj_8:25 PM\","
        "\"name\" : \"FEED 9\","
        "\"portion\" : 0.8125,"
        "\"reminder\" : false,"
        "\"time\" : 73500"
      "}, {"
        "\"active\" : true,"
        "\"automatic\" : true,"
        "\"feeding_id\" : \"40006b28dbdfd7b2_DFzLUc_9:00 PM\","
        "\"name\" : \"FEED 10\","
        "\"portion\" : 0.0625,"
        "\"reminder\" : false,"
        "\"time\" : 75600"
      "} ]"
        ;

typedef struct _test_st_ {
    bool active;
    bool automatic;
    char feeding_id[40];
    char name[10];
    float portion;
    bool reminder;
    int time;
} test_st_t;

test_st_t test_complex_array[] = {
    { true, true, "40006b28dbdfd7b2_zbxM2a_11:46 AM", "FEED 2", 0.0625, false, 42360 },
    { true, true, "40006b28dbdfd7b2_1UKZ8f_3:46 PM", "FEED 3", 0.0625, false,  56760},
    { true, true, "40006b28dbdfd7b2_YMbDPV_4:28 PM", "FEED 2", 0.0625, false,  59280},
    { true, true, "40006b28dbdfd7b2_p8HCLb_5:10 PM", "FEED 4", 0.0625, false,  61800},
    { true, true, "40006b28dbdfd7b2_ognZKK_8:15 PM", "FEED 7", 0.375, false,  72900},
    { true, true, "40006b28dbdfd7b2_Fpz5qj_8:25 PM", "FEED 9", 0.8125, false,  73500},
    { true, true, "40006b28dbdfd7b2_DFzLUc_9:00 PM", "FEED 10", 0.0625, false,  75600}
};

void test_parse_json_complex2(void) {
    JsonNode *_main = json_decode(complex2_json_text);
    TEST_ASSERT(_main);
    JsonNode *tmp = NULL;
    int count = 0;
    json_foreach(tmp, _main) {
        TEST_ASSERT(tmp);
        JsonNode *_active = json_find_member(tmp, p_const("active"));
        JsonNode *_automatic = json_find_member(tmp, p_const("automatic"));
        JsonNode *_reminder = json_find_member(tmp, p_const("reminder"));
        JsonNode *_feeding_id = json_find_member(tmp, p_const("feeding_id"));
        JsonNode *_name = json_find_member(tmp, p_const("name"));
        JsonNode *_portion = json_find_member(tmp, p_const("portion"));
        JsonNode *_time = json_find_member(tmp, p_const("time"));
        TEST_ASSERT(_active);
        TEST_ASSERT(_automatic);
        TEST_ASSERT(_reminder);
        TEST_ASSERT(_feeding_id);
        TEST_ASSERT(_name);
        TEST_ASSERT(_portion);
        TEST_ASSERT(_time);
        TEST_ASSERT( count < sizeof(test_complex_array)/sizeof(test_st_t) );
        test_st_t *st = &test_complex_array[count++];
        TEST_ASSERT(st->active == _active->bool_);
        TEST_ASSERT(st->automatic == _automatic->bool_);
        TEST_ASSERT_EQUAL_STRING(st->feeding_id, _feeding_id->string_);
        TEST_ASSERT_EQUAL_STRING(st->name, _name->string_);
        TEST_ASSERT_EQUAL_FLOAT(st->portion, _portion->number_);
        TEST_ASSERT(st->reminder == _reminder->bool_);
        TEST_ASSERT_EQUAL_FLOAT(st->time, _time->number_);
    }
    TEST_ASSERT_EQUAL_INT( 7, count );
}

void show_json_obj(JsonNode *o) {
    JsonNode *tmp = o;
    printf("tag:[%d], key:[%s], ", tmp->tag, P_VALUE(tmp->key));
    switch(tmp->tag) {
    case JSON_STRING:
        printf("v:[%s]\r\n", tmp->string_);
        break;
    case JSON_NUMBER:
        printf("v:[%d]\r\n", (int)tmp->number_);
        break;
    case JSON_BOOL:
        printf("v:[%s]\r\n", tmp->bool_?"true":"false");
        break;
    case JSON_OBJECT:
        printf("v:obj\r\n");
        json_foreach(tmp, o) {
            show_json_obj(tmp);
        }
        break;
    case JSON_ARRAY:
        printf("v:arr\r\n");
        json_foreach(tmp, o) {
            show_json_obj(tmp);
        }
        break;
    }
}

void test_parse_json_number_part(void) {
    snprintf(test, sizeof(test), JSON_INT_EX, "key", 100);
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT(_main);
    TEST_ASSERT(_key);
    TEST_ASSERT_EQUAL_INT( JSON_NUMBER, _key->tag );
    TEST_ASSERT_EQUAL_INT( 100, _key->number_ );
    json_delete(_main);
}

void test_parse_json_number_fail_part(void) {
    snprintf(test, sizeof(test), "{\"key\":12o}");
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( -1, pr );
    TEST_ASSERT(_main);
    TEST_ASSERT(!_key);
    json_delete(_main);
}

void test_parse_json_double_part(void) {
    snprintf(test, sizeof(test), "{ \"%s\":%4.2f }", "key", 3.14);
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT(_main);
    TEST_ASSERT(_key);
    TEST_ASSERT_EQUAL_INT( JSON_NUMBER, _key->tag );
    TEST_ASSERT_EQUAL_FLOAT( 3.14, _key->number_ );
    json_delete(_main);
}

void test_parse_json_number2_part(void) {
    snprintf(test, sizeof(test), "{\"k1\":%d, \"k2\":%d}", 200, 100);
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_k1 = json_find_member(_main, p_const("k1"));
    JsonNode *_k2 = json_find_member(_main, p_const("k2"));

    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT(_main);
    TEST_ASSERT(_k1);
    TEST_ASSERT(_k2);
    TEST_ASSERT_EQUAL_INT( JSON_NUMBER, _k1->tag );
    TEST_ASSERT_EQUAL_INT( 200, _k1->number_ );
    TEST_ASSERT_EQUAL_INT( 100, _k2->number_ );
    json_delete(_main);
}

void test_parse_json_number_array_part(void) {
    snprintf(test, sizeof(test), "[ { \"k\":%d }, { \"k\":%d } ]", 200, 100);
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    JsonNode *_main = json_decode_finish(&sm);
    TEST_ASSERT(_main);
    JsonNode *tmp = NULL;
    int test_arr[] = {200, 100};
    int count = 0;
    json_foreach(tmp, _main) {
        JsonNode *_k = json_find_member(tmp, p_const("k"));
        TEST_ASSERT(_k);
        TEST_ASSERT_EQUAL_INT( JSON_NUMBER, _k->tag );
        TEST_ASSERT_EQUAL_INT( test_arr[count++], _k->number_ );
    }
    json_delete(_main);
}

void test_parse_json_string_bool_array_part(void) {
    snprintf(test, sizeof(test), "[ { \"k\":\"first\",\"active\":true }, { \"k\":\"second\",\"active\":false } ]");
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    JsonNode *_main = json_decode_finish(&sm);
    TEST_ASSERT(_main);
    JsonNode *tmp = NULL;
    char *test_str_arr[] = {"first", "second"};
    bool test_bool_arr[] = {true, false};
    int count = 0;
    json_foreach(tmp, _main) {
        JsonNode *_k = json_find_member(tmp, p_const("k"));
        JsonNode *_act = json_find_member(tmp, p_const("active"));
        TEST_ASSERT(_k);
        TEST_ASSERT(_act);
        TEST_ASSERT_EQUAL_INT( JSON_STRING, _k->tag );
        TEST_ASSERT_EQUAL_INT( JSON_BOOL, _act->tag );
        TEST_ASSERT_EQUAL_STRING( test_str_arr[count], _k->string_ );
        TEST_ASSERT( test_bool_arr[count] == _act->bool_ );
        count++;
    }
    json_delete(_main);
}

void test_parse_json_bool_true_part(void) {
    snprintf(test, sizeof(test), JSON_BOOL_EX, "key", bool2str(true));
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);

    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT(_main);
    TEST_ASSERT(_key);
    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT_EQUAL_INT( JSON_BOOL, _key->tag );
    TEST_ASSERT( _key->bool_ );
}

void test_parse_json_bool_true_part_fail(void) {
    snprintf(test, sizeof(test), "{\"%s\":ture}", "key");
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT(_main);
    TEST_ASSERT(!_key);
    TEST_ASSERT_EQUAL_INT( -1, pr );
    TEST_ASSERT(!_main->children.head);
    TEST_ASSERT(!_main->children.tail);
    json_delete(_main);
}

void test_parse_json_bool_false_part(void) {
    snprintf(test, sizeof(test), JSON_BOOL_EX, "key", bool2str(false));
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT(_main);
    TEST_ASSERT(_key);
    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT_EQUAL_INT( JSON_BOOL, _key->tag );
    TEST_ASSERT( !_key->bool_ );
}

void test_parse_json_string_part(void) {
    snprintf(test, sizeof(test), JSON_STR_EX, "key", "hello");
    json_parse_machine_t sm;
    json_decode_init(&sm);
    int pr = json_decode_part(&sm, test, strlen(test));
    JsonNode *_main = json_decode_finish(&sm);
    JsonNode *_key = json_find_member(_main, p_const("key"));

    TEST_ASSERT(_main);
    TEST_ASSERT(_key);
    TEST_ASSERT_EQUAL_INT( strlen(test), pr );
    TEST_ASSERT_EQUAL_INT( JSON_STRING, _key->tag );
    TEST_ASSERT_EQUAL_INT( 0, strcmp(_key->string_, "hello") );
}

void test_parse_json_complex_part(void) {
    char test_part[100] = {0};
    int total_len = strlen(complex_json_text);
    int processed = 0;
    int rest = 0;
    json_parse_machine_t sm;
    json_decode_init(&sm);

    while (total_len) {
        int chunk = total_len < 64 - rest ? total_len : 64 - rest;
        if ( rest ) memmove(test_part, test_part + 64 - rest, rest);
        memcpy(test_part + rest, complex_json_text + processed + rest, chunk);
        int pr = json_decode_part(&sm, test_part, chunk);
        if ( pr <= 0 ) {
            break;
        }
        processed += pr;
        rest = chunk - pr;
        total_len -= pr;
    }

    JsonNode *_main = json_decode_finish(&sm);

    TEST_ASSERT(_main);
    JsonNode *_cloud = json_find_member(_main, p_const("cloudPlatform"));
    TEST_ASSERT(_cloud);
    TEST_ASSERT_EQUAL_STRING("IotConnect", _cloud->string_);
    JsonNode *_key = json_find_member(_main, p_const("key"));
    TEST_ASSERT(_key);
    JsonNode *_api = json_find_member(_key, p_const("apiKey"));
    TEST_ASSERT(_api);
    TEST_ASSERT_EQUAL_STRING("dcbd9aa24b1d86524fc54d9f721f186f70f8a3158899f2243393658bff1d4a60", _api->string_);
    JsonNode *_secret = json_find_member(_key, p_const("secretKey"));
    TEST_ASSERT(_secret);
    TEST_ASSERT_EQUAL_STRING("B41+VQRV/VcC4lnaxaFcd3rHffVZ6WdbqdxSPFP2qr8=", _secret->string_);
}

void test_parse_json_complex2_part(void) {
    char test_part[100] = {0};
    int total_len = strlen(complex2_json_text);
    int processed = 0;
    int rest = 0;
    json_parse_machine_t sm;
    json_decode_init(&sm);
    while (total_len) {
        int chunk = total_len < 64 - rest ? total_len : 64 - rest;
        if ( rest ) memmove(test_part, test_part + 64 - rest, rest);
        memcpy(test_part + rest, complex2_json_text + processed + rest, chunk);
        int pr = json_decode_part(&sm, test_part, chunk);
        if ( pr <= 0 ) {
            break;
        }
        processed += pr;
        rest = chunk - pr;
        total_len -= pr;
    }

    printf("================= %p\r\n", sm.root);
    show_json_obj(sm.root);
    printf("=================\r\n");

    JsonNode *_main = json_decode_finish(&sm);

    TEST_ASSERT(_main);
    JsonNode *tmp = NULL;
    int count = 0;
    json_foreach(tmp, _main) {
        TEST_ASSERT(tmp);
        JsonNode *_active = json_find_member(tmp, p_const("active"));
        JsonNode *_automatic = json_find_member(tmp, p_const("automatic"));
        JsonNode *_reminder = json_find_member(tmp, p_const("reminder"));
        JsonNode *_feeding_id = json_find_member(tmp, p_const("feeding_id"));
        JsonNode *_name = json_find_member(tmp, p_const("name"));
        JsonNode *_portion = json_find_member(tmp, p_const("portion"));
        JsonNode *_time = json_find_member(tmp, p_const("time"));
        TEST_ASSERT(_active);
        TEST_ASSERT(_automatic);
        TEST_ASSERT(_reminder);
        TEST_ASSERT(_feeding_id);
        TEST_ASSERT(_name);
        TEST_ASSERT(_portion);
        TEST_ASSERT(_time);
        TEST_ASSERT( count < sizeof(test_complex_array)/sizeof(test_st_t) );
        test_st_t *st = &test_complex_array[count++];
        TEST_ASSERT(st->active == _active->bool_);
        TEST_ASSERT(st->automatic == _automatic->bool_);
        TEST_ASSERT_EQUAL_STRING(st->feeding_id, _feeding_id->string_);
        TEST_ASSERT_EQUAL_STRING(st->name, _name->string_);
        TEST_ASSERT_EQUAL_FLOAT(st->portion, _portion->number_);
        TEST_ASSERT(st->reminder == _reminder->bool_);
        TEST_ASSERT_EQUAL_FLOAT(st->time, _time->number_);
    }
    TEST_ASSERT_EQUAL_INT( 7, count );
}

char json_state_req[] =
"{\"hid\":\"96c1d8101c9b925db75bceecf9ed24e08581d361\","
"\"name\":\"ServerToGateway_DeviceStateRequest\","
"\"encrypted\":false,"
"\"parameters\":{"
    "\"deviceHid\":\"86241a5e939baa7da58faff3527eb021d06d5e9a\","
    "\"transHid\":\"7115fc4c22117e6d99ac7b0a343b59db959c7b76\","
    "\"payload\":\"{\\\"delay\\\":{\\\"value\\\":\\\"1000\\\","
                                  "\\\"timestamp\\\":\\\"2018-06-03T19:35:52.931Z\\\"},"
                   "\\\"led\\\":{\\\"value\\\":\\\"false\\\","
                                "\\\"timestamp\\\":\\\"2018-06-03T19:35:52.931Z\\\"}}\""
"}"
"}";

void test_parse_json_state_req_part(void) {
    char test_part[100] = {0};
    int total_len = strlen(json_state_req);
    int processed = 0;
    int rest = 0;
    json_parse_machine_t sm;
    json_decode_init(&sm);
    while (total_len) {
        int chunk = total_len < 64 - rest ? total_len : 64 - rest;
        if ( rest ) memmove(test_part, test_part + 64 - rest, rest);
        memcpy(test_part + rest, json_state_req + processed + rest, chunk);
        int pr = json_decode_part(&sm, test_part, chunk);
        TEST_ASSERT_EQUAL_INT(chunk, pr);
        processed += pr;
        rest = chunk - pr;
        total_len -= pr;
    }

    JsonNode *_main = json_decode_finish(&sm);

    JsonNode *_hid = json_find_member(_main, p_const("hid"));
    TEST_ASSERT(_hid);
    TEST_ASSERT_EQUAL_STRING("96c1d8101c9b925db75bceecf9ed24e08581d361", _hid->string_);

    JsonNode *_name = json_find_member(_main, p_const("name"));
    TEST_ASSERT(_name);
    TEST_ASSERT_EQUAL_STRING("ServerToGateway_DeviceStateRequest", _name->string_);

    JsonNode *_enc = json_find_member(_main, p_const("encrypted"));
    TEST_ASSERT(_enc);
    TEST_ASSERT(_enc->bool_ == false);

    JsonNode *_par = json_find_member(_main, p_const("parameters"));
    TEST_ASSERT(_par);

    JsonNode *_device_hid = json_find_member(_par, p_const("deviceHid"));
    TEST_ASSERT(_device_hid);
    TEST_ASSERT_EQUAL_STRING("86241a5e939baa7da58faff3527eb021d06d5e9a", _device_hid->string_);

    JsonNode *_trans_hid = json_find_member(_par, p_const("transHid"));
    TEST_ASSERT(_trans_hid);
    TEST_ASSERT_EQUAL_STRING("7115fc4c22117e6d99ac7b0a343b59db959c7b76", _trans_hid->string_);

    JsonNode *_pay = json_find_member(_par, p_const("payload"));
    char payload_str[] = "{\"delay\":{\"value\":\"1000\",\"timestamp\":\"2018-06-03T19:35:52.931Z\"},\"led\":{\"value\":\"false\",\"timestamp\":\"2018-06-03T19:35:52.931Z\"}}";
    TEST_ASSERT(_pay);
    TEST_ASSERT_EQUAL_STRING(payload_str, _pay->string_);

    json_delete(_main);
}

char json_test_command_text[] =
"{\"hid\":\"3a7f0d3099d1bbc78aa7d16f6530f67f56c5c282\","
"\"name\":\"ServerToGateway_DeviceCommand\","
"\"encrypted\":false,"
"\"parameters\":{\"deviceHid\":\"86241a5e939baa7da58faff3527eb021d06d5e9a\","
                "\"command\":\"test\","
                "\"payload\":\"{\n\t}\"},"
"\"signature\":\"02e1909a335edbcb6471c4ec1273622344e49bea3b9566549d1e04f24fc65e71\","
"\"signatureVersion\":\"1\"}";

void test_parse_json_command_dev_part(void) {
    char test_part[100] = {0};
    int total_len = strlen(json_test_command_text);
    int processed = 0;
    int rest = 0;
    json_parse_machine_t sm;
    json_decode_init(&sm);
    while (total_len) {
        int chunk = total_len < 64 - rest ? total_len : 64 - rest;
        if ( rest ) memmove(test_part, test_part + 64 - rest, rest);
        memcpy(test_part + rest, json_test_command_text + processed + rest, chunk);
        int pr = json_decode_part(&sm, test_part, chunk);
        TEST_ASSERT_EQUAL_INT(chunk, pr);
        processed += pr;
        rest = chunk - pr;
        total_len -= pr;
    }

    JsonNode *_main = json_decode_finish(&sm);

    JsonNode *_hid = json_find_member(_main, p_const("hid"));
    TEST_ASSERT(_hid);
    TEST_ASSERT_EQUAL_STRING("3a7f0d3099d1bbc78aa7d16f6530f67f56c5c282", _hid->string_);

    JsonNode *_name = json_find_member(_main, p_const("name"));
    TEST_ASSERT(_name);
    TEST_ASSERT_EQUAL_STRING("ServerToGateway_DeviceCommand", _name->string_);

    JsonNode *_enc = json_find_member(_main, p_const("encrypted"));
    TEST_ASSERT(_enc);
    TEST_ASSERT(_enc->bool_ == false);

    JsonNode *_par = json_find_member(_main, p_const("parameters"));
    TEST_ASSERT(_par);

    JsonNode *_device_hid = json_find_member(_par, p_const("deviceHid"));
    TEST_ASSERT(_device_hid);
    TEST_ASSERT_EQUAL_STRING("86241a5e939baa7da58faff3527eb021d06d5e9a", _device_hid->string_);

    JsonNode *_command = json_find_member(_par, p_const("command"));
    TEST_ASSERT(_command);
    TEST_ASSERT_EQUAL_STRING("test", _command->string_);

    JsonNode *_pay = json_find_member(_par, p_const("payload"));
    char payload_str[] = "{\n\t}";
    TEST_ASSERT(_pay);
    TEST_ASSERT_EQUAL_STRING(payload_str, _pay->string_);

    json_delete(_main);
}

void test_encode_json_number(void) {
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mknumber(100);
    json_append_member(_main, p_const("key"), _key);

    char *s = json_encode(_main);

    TEST_ASSERT_EQUAL_STRING("{\"key\":100}", s);
    free(s);
}

void test_encode_json_string(void) {
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkstring("value");
    json_append_member(_main, p_const("key"), _key);

    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":\"value\"}", s);
    free(s);
}

void test_encode_json_bool_true(void) {
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkbool(true);
    json_append_member(_main, p_const("key"), _key);

    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":true}", s);
    free(s);
}

void test_encode_json_bool_false(void) {
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkbool(false);
    json_append_member(_main, p_const("key"), _key);

    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":false}", s);
    free(s);
}

void test_encode_json_array(void) {
    JsonNode *_main = json_mkarray();
    JsonNode *_key1 = json_mkobject();
    json_append_member(_key1, p_const("k1"), json_mknumber(100));
    JsonNode *_key2 = json_mkobject();
    json_append_member(_key2, p_const("k2"), json_mknumber(200));
    json_append_element(_main, _key1);
    json_append_element(_main, _key2);

    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_STRING("[{\"k1\":100},{\"k2\":200}]", s);
    free(s);
}

void test_encode_json_number_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mknumber(100);
    json_append_member(_main, p_const("key"), _key);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
        if(total > 50) break;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":100}", test);
}

void test_encode_json_number2_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_k1 = json_mknumber(100);
    json_append_member(_main, p_const("k1"), _k1);
    JsonNode *_k2 = json_mknumber(200);
    json_append_member(_main, p_const("k2"), _k2);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 2);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"k1\":100,\"k2\":200}", test);
}

void test_encode_json_number5_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_k1 = json_mknumber(100);
    json_append_member(_main, p_const("k1"), _k1);
    JsonNode *_k2 = json_mknumber(200);
    json_append_member(_main, p_const("k2"), _k2);
    JsonNode *_k3 = json_mknumber(3.1);
    json_append_member(_main, p_const("k3"), _k3);
    JsonNode *_k4 = json_mknumber(400.5);
    json_append_member(_main, p_const("k4"), _k4);
    JsonNode *_k5 = json_mknumber(5000);
    json_append_member(_main, p_const("k5"), _k5);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"k1\":100,\"k2\":200,\"k3\":3.1,\"k4\":400.5,\"k5\":5000}", test);
}

void test_encode_json_bool_true_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkbool(true);
    json_append_member(_main, p_const("key"), _key);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":true}", test);
}

void test_encode_json_bool_false_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkbool(false);
    json_append_member(_main, p_const("key"), _key);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"key\":false}", test);
}

void test_encode_json_bool_bool2_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_k1 = json_mkbool(true);
    json_append_member(_main, p_const("k1"), _k1);
    JsonNode *_k2 = json_mkbool(false);
    json_append_member(_main, p_const("k2"), _k2);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    json_delete(_main);
    TEST_ASSERT_EQUAL_STRING("{\"k1\":true,\"k2\":false}", test);
}

void test_encode_json_string_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mkstring("value");
    json_append_member(_main, p_const("key"), _key);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);

    TEST_ASSERT_EQUAL_STRING("{\"key\":\"value\"}", test);
    json_delete(_main);
}

void test_encode_json_string3_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    JsonNode *_k1 = json_mkstring("value1");
    JsonNode *_k2 = json_mkstring("value2");
    JsonNode *_k3 = json_mkstring("value3");
    json_append_member(_main, p_const("k1"), _k1);
    json_append_member(_main, p_const("k2"), _k2);
    json_append_member(_main, p_const("k3"), _k3);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);
    TEST_ASSERT_EQUAL_STRING("{\"k1\":\"value1\",\"k2\":\"value2\",\"k3\":\"value3\"}", test);
    json_delete(_main);
}

void test_encode_json_array_part(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkarray();
    JsonNode *_el1 = json_mkobject();
    JsonNode *_el2 = json_mkobject();
    JsonNode *_el3 = json_mkobject();
    json_append_member(_el1, p_const("k1"), json_mkstring("val1"));
    json_append_member(_el2, p_const("k2"), json_mkstring("val1"));
    json_append_member(_el3, p_const("k3"), json_mkstring("val1"));
    json_append_element(_main, _el1);
    json_append_element(_main, _el2);
    json_append_element(_main, _el3);

    int r = json_encode_init(&em, _main);
    TEST_ASSERT_EQUAL_INT(0, r);
    int total = 0;
    char encode_test[64];
    memset(test, 0x0, 64);
    do {
        r = json_encode_part(&em, encode_test, 1);
        if ( r <= 0 ) break;
        memcpy(test + total, encode_test, r);
        total += r;
    } while(r > 0);
    test[total] = 0x0;
    TEST_ASSERT_EQUAL_INT(0, r);
    r = json_encode_fin(&em);

    char *pattern = json_encode(_main);
    TEST_ASSERT_EQUAL_STRING(pattern, test);
    free(pattern);
    json_delete(_main);
}

void test_size_json_number(void) {
    JsonNode *_main = json_mkobject();
    JsonNode *_key = json_mknumber(100);
    json_append_member(_main, p_const("key"), _key);

    size_t size = json_size(_main);
    char *s = json_encode(_main);

    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_number2(void) {
    json_encode_machine_t em;
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("k1"), json_mknumber(100));
    json_append_member(_main, p_const("k2"), json_mknumber(200.1));

    size_t size = json_size(_main);
    char *s = json_encode(_main);

    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}


void test_size_json_bool_true(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("key"), json_mkbool(true));

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_bool_false(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("key"), json_mkbool(false));

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_bool2(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("k1"), json_mkbool(true));
    json_append_member(_main, p_const("k2"), json_mkbool(false));

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_string(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("key"), json_mkstring("value1"));

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_string2(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("key"), json_mkstring("value1"));
    json_append_member(_main, p_const("k2"), json_mkstring("value2"));

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_string_obj(void) {
    JsonNode *_main = json_mkobject();
    json_append_member(_main, p_const("key"), json_mkstring("value1"));
    JsonNode *t = json_mkobject();
    json_append_member(t, p_const("additional"), json_mknumber(100));
    json_append_member(_main, p_const("k2"), t);

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}

void test_size_json_array(void) {
    JsonNode *_main = json_mkarray();
    JsonNode *_el1 = json_mkobject();
    JsonNode *_el2 = json_mkobject();
    JsonNode *_el3 = json_mkobject();
    json_append_member(_el1, p_const("k1"), json_mkstring("val1"));
    json_append_member(_el2, p_const("k2"), json_mkstring("val1"));
    json_append_member(_el3, p_const("k3"), json_mkstring("val1"));
    json_append_element(_main, _el1);
    json_append_element(_main, _el2);
    json_append_element(_main, _el3);

    size_t size = json_size(_main);
    char *s = json_encode(_main);
    TEST_ASSERT_EQUAL_INT(strlen(s), size);
    free(s);
    json_delete(_main);
}
