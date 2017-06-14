#if !defined(ARROW_SOFTWARE_UPDATE_H_)
#define ARROW_SOFTWARE_UPDATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/connection.h>

int ev_GatewaySoftwareUpdate(void *_ev, JsonNode *_parameters);
int arrow_gateway_software_update(const char *url);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_SOFTWARE_UPDATE_H_
