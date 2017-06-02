#include "arrow/testsuite.h"

#include <debug.h>

#define URI_LEN sizeof(ARROW_API_DEVICE_ENDPOINT) + 50

static void _test_gateway_init(http_request_t *request, void *arg) {
  arrow_gateway_t *gate = (arrow_gateway_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/gateways/test", ARROW_API_TESTSUITE_ENDPOINT);
  http_request_init(request, POST, uri);
  free(uri);
  char *payload = arrow_gateway_serialize(gate);
  http_request_set_payload_ptr(request, payload);
  DBG("gate|%s|", payload);
}

static int _test_gateway_proc(http_response_t *response, void *arg) {
  DBG("TEST res", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  return 0;
}

int arrow_test_gateway(arrow_gateway_t *gateway) {
  int ret = __http_routine(_test_gateway_init, gateway, _test_gateway_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST gateway failed...");
  }
  return ret;
}

static void _test_device_init(http_request_t *request, void *arg) {
  arrow_device_t *dev = (arrow_device_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/devices/test", ARROW_API_TESTSUITE_ENDPOINT);
  http_request_init(request, POST, uri);
  free(uri);
  char *payload = arrow_device_serialize(dev);
  http_request_set_payload_ptr(request, payload);
  DBG("dev|%s|", payload);
}

int arrow_test_device(arrow_device_t *device) {
  int ret = __http_routine(_test_device_init, device, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST device failed...");
  }
  return ret;
}

static void _test_begin_init(http_request_t *request, void *arg) {
  const char *hid = (const char *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/begin", ARROW_API_TESTSUITE_ENDPOINT, hid);
  http_request_init(request, POST, uri);
  free(uri);
}

int arrow_test_begin(const char *hid) {
  int ret = __http_routine(_test_begin_init, (void*)hid, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST begin failed...");
  }
  return ret;
}

typedef struct _test_step_ {
  const char *hid;
  int number;
} test_step_t;

static void _test_step_begin_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/begin",
           ARROW_API_TESTSUITE_ENDPOINT,
           st->hid,
           st->number);
  http_request_init(request, POST, uri);
  free(uri);
}

int arrow_test_step_begin(const char *hid, int number) {
  test_step_t st = {hid, number};
  int ret = __http_routine(_test_step_begin_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST begin step failed...");
  }
  return ret;
}

static void _test_step_end_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/end",
           ARROW_API_TESTSUITE_ENDPOINT,
           st->hid,
           st->number);
  http_request_init(request, POST, uri);
  free(uri);
}

int arrow_test_step_end(const char *hid, int number) {
  test_step_t st = {hid, number};
  int ret = __http_routine(_test_step_end_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST end step failed...");
  }
  return ret;
}

static void _test_step_skip_init(http_request_t *request, void *arg) {
  test_step_t *st = (test_step_t *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/steps/%d/skip",
           ARROW_API_TESTSUITE_ENDPOINT,
           st->hid,
           st->number);
  http_request_init(request, POST, uri);
  free(uri);
}

int arrow_test_step_skip(const char *hid, int number) {
  test_step_t st = {hid, number};
  int ret = __http_routine(_test_step_skip_init, &st, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST skip step failed...");
  }
  return ret;
}

static void _test_end_init(http_request_t *request, void *arg) {
  const char *hid = (const char *)arg;
  char *uri = (char *)malloc(URI_LEN);
  snprintf(uri, URI_LEN, "%s/tests/%s/end", ARROW_API_TESTSUITE_ENDPOINT, hid);
  http_request_init(request, POST, uri);
  free(uri);
}

int arrow_test_end(const char *hid) {
  int ret = __http_routine(_test_end_init, (void*)hid, NULL, NULL);
  if ( ret < 0 ) {
    DBG("Arrow TEST end failed...");
  }
  return ret;
}
