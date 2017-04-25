#if !defined(ARROW_DEVICECOMMAND_H_)
#define ARROW_DEVICECOMMAND_H_

#include <json/json.h>

typedef enum {
    failed,
    received,
    succeeded
} cmd_type;

typedef enum {
    CMD_OK,
    CMD_NO_HANDLER = -1,
    CMD_ERROR = -2
} handler_result;


typedef int (*fp)(const char *);

typedef struct __cmd_handler {
  char *name;
  fp callback;
  struct __cmd_handler *next;
} cmd_handler;

int add_cmd_handler(const char *name, fp callback);
int ev_DeviceCommand(void *ev, JsonNode *node);

int arrow_send_event_ans(const char *hid, cmd_type ev, const char *payload);

#endif // ARROW_DEVICECOMMAND_H_
