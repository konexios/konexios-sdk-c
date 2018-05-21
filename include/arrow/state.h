#if !defined(ARROW_STATE_H_)
#define ARROW_STATE_H_

#include <arrow/device.h>

// set a new device state
void arrow_device_state_add_string(property_t name,
                                   const char *value);
void arrow_device_state_add_number(property_t name,
                                   int value);
void arrow_device_state_add_bool(property_t name,
                                 bool value);

// get device state information
int arrow_state_receive(arrow_device_t *device);
int arrow_post_state_request(arrow_device_t *device);
int arrow_post_state_update(arrow_device_t *device);

// allow to use
int arrow_state_mqtt_run(arrow_device_t *device);
int arrow_state_mqtt_is_running(void);
int arrow_state_mqtt_stop(void);

// DeviceStateRequest event handler
int ev_DeviceStateRequest(void *, JsonNode *_parameters);

int arrow_device_state_handler(char *str);

#endif // ARROW_STATE_H_
