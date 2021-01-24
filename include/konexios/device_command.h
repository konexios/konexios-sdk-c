#if !defined(KONEXIOS_DEVICE_COMMAND_H_)
#define KONEXIOS_DEVICE_COMMAND_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <json/json.h>
#include <data/linkedlist.h>

typedef enum {
    cmd_failed,
    cmd_received,
    cmd_succeeded
} cmd_type;

typedef enum {
    CMD_OK,
    CMD_NO_HANDLER = -1,
    CMD_ERROR = -2
} handler_result;


typedef int (*__cmd_cb)(property_t);

typedef struct __cmd_handler {
  property_t name;
  __cmd_cb callback;
  konexios_linked_list_head_node;
} cmd_handler;

// Is there any command handler was added
int has_cmd_handler(void);

// Add a new command handler ( set the callback )
int konexios_command_handler_add(const char *name, __cmd_cb callback);

// erase all command handlers
void konexios_command_handler_free(void);

// DeviceCommand event handler
int ev_DeviceCommand(void *ev, JsonNode *node);

// send the answer by DeviceCommand: failed, received or succeeded (cmd_type)
int konexios_send_event_ans(property_t hid, cmd_type ev, property_t payload);

#if defined(__cplusplus)
}
#endif

#endif // KONEXIOS_DEVICE_COMMAND_H_
