#include "json/telemetry.h"
#include <config.h>
#include <json/json.h>


char *telemetry_serialize(arrow_device_t *device, sensor_data_t *data) {
    JsonNode *_node = json_mkobject();
    probook_data_t *pro_data = (probook_data_t *)data;
    json_append_member(_node, TELEMETRY_DEVICE_HID, json_mkstring(device->hid));
    json_append_member(_node, "f|Core0Temperature", json_mknumber(pro_data->temperature_core0));
    json_append_member(_node, "f|Core1Temperature", json_mknumber(pro_data->temperature_core1));
    char *tmp = json_encode(_node);
    json_delete(_node);
    return tmp;
}
