#if !defined(ARROW_SOFTWARE_UPDATE_H_)
#define ARROW_SOFTWARE_UPDATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

typedef int (*__update_cb)(const char *url);

int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters);
int arrow_gateway_software_update(const char *url);
int arrow_gateway_software_update_set_cb(__update_cb);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_SOFTWARE_UPDATE_H_
