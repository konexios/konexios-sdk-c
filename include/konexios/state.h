#if !defined(ARROW_STATE_H_)
#define ARROW_STATE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <konexios/device.h>

typedef struct _state_pair_ {
    property_t name;
    int typetag;
//#if defined(__cplusplus)
//    _state_pair_(property_t n, int t) : typetag(t) {
//        property_copy(&name, n);
//    }
//#endif
} konexios_state_pair_t;

#if defined(__cplusplus)
konexios_state_pair_t state_pr(property_t x, int y);
#else
#define state_pr(x, y) (konexios_state_pair_t) { (x), (y) }
#endif

// set a new device state with default value
void konexios_device_state_init(int n, ...);
void konexios_device_state_set_string(property_t name,
                                   property_t value);
void konexios_device_state_set_number(property_t name,
                                   int value);
void konexios_device_state_set_bool(property_t name,
                                 bool value);

void konexios_device_state_free(void);

// get device state information
int konexios_state_receive(konexios_device_t *device);
int konexios_post_state_request(konexios_device_t *device);
int konexios_post_state_update(konexios_device_t *device);

// allow to use
int konexios_state_mqtt_is_running(void);
int konexios_state_mqtt_run(konexios_device_t *device);
int konexios_state_mqtt_stop(void);
int konexios_state_deinit(void);

// DeviceStateRequest event handler
int ev_DeviceStateRequest(void *, JsonNode *_parameters);

int konexios_device_state_handler(JsonNode *states);

#if defined(__cplusplus)
}
#endif

#endif // ARROW_STATE_H_
