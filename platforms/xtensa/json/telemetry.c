#include "json/telemetry.h"
#include <debug.h>
char *telemetry_serialize(arrow_device_t *device, sensor_data_t *data) {
    JsonNode *_node = json_mkobject();
    rssi_data_t *qca_data = (rssi_data_t *)data;
    json_append_member(_node, TELEMETRY_DEVICE_HID, json_mkstring(device->hid));
    json_append_member(_node, "i|rssi", json_mknumber(qca_data->rssi));
    json_append_member(_node, TELEMETRY_TEMPERATURE, json_mknumber(qca_data->temperature));
    char *tmp = json_encode(_node);
    json_minify(tmp);
    DBG("telemetry|%s|", tmp);
    json_delete(_node);
    return tmp;
}
