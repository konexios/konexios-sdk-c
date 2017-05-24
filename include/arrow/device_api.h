#if !defined(ARROW_DEVICE_API_H_)
#define ARROW_DEVICE_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/find_by.h>
#include <arrow/connection.h>

int arrow_register_device(arrow_gateway_t *gateway, arrow_device_t *device);
int arrow_device_find_by(int n, ...);
int arrow_device_find_by_hid(const char *hid);
int arrow_update_device(arrow_gateway_t *gateway, arrow_device_t *device);
int arrow_list_device_events(arrow_device_t *device, int n, ...);
int arrow_list_device_logs(arrow_device_t *device, int n, ...);
int arrow_error_device(arrow_device_t *device, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_DEVICE_API_H_
