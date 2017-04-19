#if !defined(ARROW_EVENTS_H_)
#define ARROW_EVENTS_H_


typedef enum {
    failed,
    received,
    succeeded
} event_t;

typedef enum {
    CMD_OK,
    CMD_NO_HANDLER = -1,
    CMD_ERROR = -2
} handler_t;



typedef int (*fp)(const char *);

typedef struct __cmd_handler {
  char *name;
  fp callback;
  struct __cmd_handler *next;
} cmd_handler;

int add_cmd_handler(const char *name, fp callback);

int arrow_send_event_ans(const char *hid, event_t ev, const char *payload);

int process_event(const char *str);

#endif // ARROW_EVENTS_H_
