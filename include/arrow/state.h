#if !defined(ARROW_STATE_H_)
#define ARROW_STATE_H_

#include <arrow/device.h>

void add_state(const char *name, const char *value);

int arrow_get_state(arrow_device_t *device);
int arrow_post_state_request(arrow_device_t *device);
int arrow_post_state_update(arrow_device_t *device);

#endif // ARROW_STATE_H_
