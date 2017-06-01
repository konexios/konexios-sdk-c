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
  DBG("dev|%s|", payload);
}

static int _test_gateway_proc(http_response_t *response, void *arg) {
  DBG("account res", response->m_httpResponseCode);
  if ( response->m_httpResponseCode != 200 )
    return -1;
  return 0;
}

int arrow_test_gateway(arrow_gateway_t *gateway) {
  int ret = __http_routine(_test_gateway_init, gateway, _test_gateway_proc, NULL);
  if ( ret < 0 ) {
    DBG("Arrow Account create failed...");
  }
  return ret;
}
