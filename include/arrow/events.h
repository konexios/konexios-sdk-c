#if !defined(ARROW_EVENTS_H_) && !defined(NO_EVENTS)
#define ARROW_EVENTS_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <json/json.h>
#include <data/linkedlist.h>

typedef struct {
  property_t gateway_hid;
  property_t device_hid;
  property_t name;
  int encrypted;
  char *cmd;
  JsonNode *parameters;
  arrow_linked_list_head_node;
} mqtt_event_t;

void arrow_mqtt_events_init(void);
void arrow_mqtt_events_done(void);
int process_event_init();
int process_event(const char *str, int len);
int process_event_finish();
int arrow_mqtt_has_events(void);
int arrow_mqtt_event_proc(void);

#define MAX_PARAM_LINE 20

#if defined(__cplusplus)
}
#endif

#endif // ARROW_EVENTS_H_
