#include "konexios/testsuite.h"
#include <http/routine.h>
#include <debug.h>
#include <data/chunk.h>
#include <json/decode.h>

#define URI_LEN sizeof(KONEXIOS_API_TESTSUITE_ENDPOINT) + 256

//gateways

static void _test_gateway_init(http_request_t *request, void *arg) {
  konexios_gateway_t *pg = (konexios_gateway_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/gateways/%s", KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(pg->hid));
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

static int _test_gateway_proc(http_response_t *response, void *arg) {
  property_t *res_hid = (property_t *)arg;
  if ( response->m_httpResponseCode != 200 ) return -1;
  JsonNode *_main = json_decode_property(response->payload);
  if ( !_main ) return -1;
  JsonNode *test_res = json_find_member(_main, p_const("hid"));
  if ( !test_res ) {
    json_delete(_main);
    return -1;
  }
  DBG("test result hid %s", json_string(test_res));
  property_copy(res_hid, test_res->string_);
  return 0;
}

int konexios_test_gateway(konexios_gateway_t *gateway, property_t *res_hid) {
  STD_ROUTINE(_test_gateway_init, gateway,
              _test_gateway_proc, res_hid,
              "Arrow TEST gateway failed...");
}

typedef struct _proc_gate_ {
  konexios_gateway_t *gate;
  const char *value;
} proc_gate_t;

static void _test_gateway_hid_init(http_request_t *request, void *arg) {
  proc_gate_t *pg = (proc_gate_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/gateways/%s/test-procedures/hids/%s",
                     KONEXIOS_API_TESTSUITE_ENDPOINT,
                     P_VALUE(pg->gate->hid),
                     pg->value);
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_gateway_proc_hid(konexios_gateway_t *gateway, const char *phid, property_t *res_hid) {
    proc_gate_t pg = { gateway, phid };
    STD_ROUTINE(_test_gateway_hid_init, &pg,
                _test_gateway_proc, res_hid,
                "Arrow TEST gateway HID failed...");
}

static void _test_gateway_name_init(http_request_t *request, void *arg) {
  proc_gate_t *pg = (proc_gate_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/gateways/%s/test-procedures/names/%s",
                     KONEXIOS_API_TESTSUITE_ENDPOINT,
                     P_VALUE(pg->gate->hid),
                     pg->value);
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_gateway_proc_name(konexios_gateway_t *gateway, const char *name, property_t *res_hid) {
    proc_gate_t pg = { gateway, name };
    STD_ROUTINE(_test_gateway_name_init, &pg,
                _test_gateway_proc, res_hid,
                "Arrow TEST gateway NAME failed...");
}

// devices

static void _test_device_init(http_request_t *request, void *arg) {
  konexios_device_t *dev = (konexios_device_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/devices/%s", KONEXIOS_API_TESTSUITE_ENDPOINT, P_VALUE(dev->hid));
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

static int _test_device_proc(http_response_t *response, void *arg) {
  property_t *res_his = (property_t *)arg;
  if ( response->m_httpResponseCode != 200 ) {
      if ( !IS_EMPTY(response->payload) ) {
          DBG("TEST FAILED [%s]", P_VALUE(response->payload));
      }
      return -1;
  }
  JsonNode *_main = json_decode_property(response->payload);
  if ( !_main ) return -1;
  JsonNode *test_res = json_find_member(_main, p_const("hid"));
  if ( !test_res ) {
    json_delete(_main);
    return -1;
  }
  DBG("test result hid %s", json_string(test_res));
  property_copy(res_his, test_res->string_);
  return 0;
}

int konexios_test_device(konexios_device_t *device, property_t *res_his) {
  STD_ROUTINE(_test_device_init, device,
              _test_device_proc, res_his,
              "Arrow TEST device failed...");
}

typedef struct _proc_dev_ {
  konexios_device_t *dev;
  const char *value;
} proc_dev_t;

static void _test_device_proc_hid_init(http_request_t *request, void *arg) {
  proc_dev_t *pd = (proc_dev_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/devices/%s/test-procedures/hids/%s",
                     KONEXIOS_API_TESTSUITE_ENDPOINT,
                     P_VALUE(pd->dev->hid),
                     pd->value);
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_device_proc_hid(konexios_device_t *device, const char *phid, property_t *res_hid) {
    proc_dev_t pd = { device, phid };
    STD_ROUTINE(_test_device_proc_hid_init, &pd,
                _test_device_proc, res_hid,
                "TEST Device proc HID failed...");
}

static void _test_device_proc_name_init(http_request_t *request, void *arg) {
  proc_dev_t *pd = (proc_dev_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/devices/%s/test-procedures/names/%s",
                     KONEXIOS_API_TESTSUITE_ENDPOINT,
                     P_VALUE(pd->dev->hid),
                     pd->value);
  uri[ret] = 0x0;
  http_request_init(request, POST, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_device_proc_name(konexios_device_t *device, const char *name, property_t *res_hid) {
    proc_dev_t pd = { device, name };
    STD_ROUTINE(_test_device_proc_name_init, &pd,
                _test_device_proc, res_hid,
                "TEST Device proc NAME failed...");
}

// tests

static void _test_begin_init(http_request_t *request, void *arg) {
  property_t *res_hid = (property_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/begin", KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*res_hid));
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_begin(property_t *res_hid) {
  STD_ROUTINE(_test_begin_init, res_hid,
              NULL, NULL,
              "Arrow TEST begin failed...");
}

static void _test_end_init(http_request_t *request, void *arg) {
  property_t *res_hid = (property_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/end",
           KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*res_hid));
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_end(property_t *res_hid) {
  STD_ROUTINE(_test_end_init, res_hid,
              NULL, NULL,
              "Arrow TEST end failed...");
}

typedef struct _test_step_ {
  property_t *res_hid;
  int number;
} test_step_t;

static void _test_step_begin_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/begin",
           KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*st->res_hid),
           st->number);
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_step_begin(property_t *res_hid, int number) {
  test_step_t st = { res_hid, number };
  STD_ROUTINE(_test_step_begin_init, &st,
              NULL, NULL,
              "Arrow TEST begin step failed...");
}

typedef struct _test_step_success_end_ {
  property_t *res_hid;
  int number;
} test_step_success_end_t;

static void _test_step_success_end_init(http_request_t *request, void *arg) {
  test_step_success_end_t *st = (test_step_success_end_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/succeeded",
           KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*st->res_hid),
           st->number);
  uri[ret] = 0x0;
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_step_success(property_t *res_hid, int number) {
    test_step_success_end_t st = { res_hid, number };
    STD_ROUTINE(_test_step_success_end_init, &st,
                NULL, NULL,
                "Arrow TEST end step failed...");
}

typedef struct _test_step_fail_end_ {
  property_t *res_hid;
  int number;
  const char *code;
  const char *error;
} test_step_fail_end_t;

static void _test_step_fail_end_init(http_request_t *request, void *arg) {
  test_step_fail_end_t *st = (test_step_fail_end_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  int ret = snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/failed",
           KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*st->res_hid),
           st->number);
  uri[ret] = 0x0;
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
  JsonNode *_main = json_mkobject();
  if ( st->code )
      json_append_member(_main, p_const("code"), json_mkstring(st->code));
  if ( st->error )
      json_append_member(_main, p_const("error"), json_mkstring(st->error));
  http_request_set_payload(request, json_encode_property(_main));
  json_delete(_main);
}

int konexios_test_step_fail(property_t *res_hid, int number, const char *error) {
    test_step_fail_end_t st = { res_hid, number, NULL, error };
    STD_ROUTINE(_test_step_fail_end_init, &st,
                NULL, NULL,
                "Arrow TEST end step failed...");
}

static void _test_step_skip_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  CREATE_CHUNK(uri, URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/skip",
           KONEXIOS_API_TESTSUITE_ENDPOINT,
           P_VALUE(*st->res_hid),
           st->number);
  http_request_init(request, PUT, &p_stack(uri));
  FREE_CHUNK(uri);
}

int konexios_test_step_skip(property_t *res_hid, int number) {
  test_step_t st = {res_hid, number};
  STD_ROUTINE(_test_step_skip_init, &st,
              NULL, NULL,
              "Arrow TEST skip step failed...");
}

