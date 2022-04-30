#include "konexios/software_update.h"
#include <debug.h>
#include <konexios/events.h>
#include <konexios/routine.h>
#include <konexios/api/gateway/gateway.h>
#include <time/time.h>
#include "sys/reboot.h"

#if defined(NO_SOFTWARE_UPDATE)
typedef void __dummy__;
#else
__update_cb __attribute__((weak)) __update;

int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters) {
  SSP_PARAMETER_NOT_USED(_ev);
  DBG("start software update processing");
//  mqtt_event_t *ev = (mqtt_event_t *)_ev;
  JsonNode *tmp = json_find_member(_parameters, p_const("url"));
  if ( !tmp || tmp->tag != JSON_STRING ) return -1;
  DBG("update url: %s", P_VALUE(tmp->string_));

  if ( konexios_gateway_software_update(P_VALUE(tmp->string_)) < 0 ) return -1;
  DBG("Reboot...");
  reboot(0);
  return 0;
}

int __attribute__((weak)) konexios_gateway_software_update(const char *url) {
  if ( __update ) return __update(url);
  return -1;
}

int konexios_gateway_software_update_set_cb(__update_cb cb) {
  __update = cb;
  return 0;
}
#endif
