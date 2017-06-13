#include "arrow/software_update.h"
#include <debug.h>
#include <arrow/events.h>

int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters) {
  DBG("start software update processing");
  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  JsonNode *tmp = json_find_member(_parameters, "url");
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  DBG("update url: %s", tmp->string_);

  return arrow_gateway_software_update(tmp->string_);
}

int __attribute__((weak)) arrow_gateway_software_update(const char *url) {
  SSP_PARAMETER_NOT_USED(url);
  return 0;
}
