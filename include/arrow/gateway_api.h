#if !defined(ARROW_GATEWAY_API_H_)
#define ARROW_GATEWAY_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/find_by.h>
#include <arrow/connection.h>

int arrow_register_gateway(arrow_gateway_t *gateway);
int arrow_gateway_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config);

int arrow_gateway_find(const char *hid);
int arrow_gateway_find_by(int n, ...);
int arrow_gateway_logs_list(arrow_gateway_t *gateway, int n, ...);
int arrow_gateway_devices_list(const char *hid);
int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload);
int arrow_gateway_update(arrow_gateway_t *gateway);
int arrow_gateway_error(arrow_gateway_t *gateway, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_GATEWAY_API_H_
