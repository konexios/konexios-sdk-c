#if !defined(ARROW_GATEWAY_API_H_)
#define ARROW_GATEWAY_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <data/find_by.h>
#include <arrow/gateway.h>
#include <arrow/device.h>
#include <time/time.h>

enum {
    GATEWAY_ERROR           = -100,
    GATEWAY_REGISTER_ERROR  = -101,
    GATEWAY_HEARTBEAT_ERROR = -102,
    GATEWAY_CHECKIN_ERROR   = -103,
    GATEWAY_CONFIG_ERROR    = -104,
    GATEWAY_FIND_ERROR      = -105,
    GATEWAY_FINDBY_ERROR    = -106,
    GATEWAY_LOGS_ERROR      = -107,
    GATEWAY_DEVLIST_ERROR   = -108,
    GATEWAY_DEVCOMS_ERROR   = -109,
    GATEWAY_UPDATE_ERROR    = -110
};

typedef struct _gateway_info_ {
    property_t hid;
    struct tm createdDate;
    property_t createdBy;
    struct tm lastModifiedDate;
    property_t lastModifiedBy;
    property_t uid;
    property_t name;
    property_t type;
    property_t deviceType;
    property_t osName;
    property_t softwareName;
    property_t softwareVersion;
    linked_list_head_node;
} gateway_info_t;

void gateway_info_init(gateway_info_t *gi);
void gateway_info_free(gateway_info_t *gi);

// register new gateway
int arrow_register_gateway(arrow_gateway_t *gateway);
// download gateway configuration
int arrow_gateway_config(arrow_gateway_t *gateway, arrow_gateway_config_t *config);
// send the heartbeat request
int arrow_gateway_heartbeat(arrow_gateway_t *gateway);
// send the checkin request
int arrow_gateway_checkin(arrow_gateway_t *gateway);
// find gateway by hid
int arrow_gateway_find(const char *hid);
// find gateway by other any parameters
int arrow_gateway_find_by(gateway_info_t **info, int n, ...);
// list gateway audit logs
int arrow_gateway_logs_list(arrow_gateway_t *gateway, int n, ...);
// list gateway devices
int arrow_gateway_devices_list(const char *hid);
// send command and payload to gateway and device
int arrow_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload);
// update existing gateway
int arrow_gateway_update(arrow_gateway_t *gateway);
// send the error request
int arrow_gateway_error(arrow_gateway_t *gateway, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // ARROW_GATEWAY_API_H_
