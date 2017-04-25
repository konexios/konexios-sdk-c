#if !defined(ARROW_EVENTS_H_)
#define ARROW_EVENTS_H_

typedef struct {
  char *gateway_hid;
  char *device_hid;
  char *cmd;
  char *payload;
} mqtt_event_t;

int process_event(const char *str);

#endif // ARROW_EVENTS_H_
