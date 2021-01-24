#if !defined(KONEXIOS_EVENTS_H_) && !defined(NO_EVENTS)
#define KONEXIOS_EVENTS_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <json/json.h>
#include <data/linkedlist.h>
#include <http/response.h>
#include <konexios/device_command.h>

typedef struct {
    property_t id;
    property_t name;
    int encrypted;
    JsonNode *node;
    JsonNode *parameters;
} mqtt_event_base_t;

typedef struct {
  mqtt_event_base_t base;
  konexios_linked_list_head_node;
} mqtt_event_t;

typedef struct {
  mqtt_event_base_t base;
  konexios_linked_list_head_node;
} mqtt_api_event_t;

void konexios_mqtt_events_init(void);
void konexios_mqtt_events_done(void);
int process_event_init(int size);
int process_event(const char *str, int len);
int process_event_finish();

int process_http_init(int size);
int process_http(const char *str, int len);
int process_http_finish();

int konexios_mqtt_has_events(void);
int konexios_mqtt_event_proc(void);

int konexios_mqtt_api_send(mqtt_event_t *event, cmd_type status);
int konexios_mqtt_api_wait(int num);
int konexios_mqtt_api_has_events(void);
int konexios_mqtt_api_event_proc(http_response_t *res);
int process_http_payload(const char *s);

#if defined(__cplusplus)
}
#endif

#endif // KONEXIOS_EVENTS_H_
