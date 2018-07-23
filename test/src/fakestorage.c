#include "fakestorage.h"
#include <string.h>

static const char *d_hid = NULL;
static const char *g_hid = NULL;

void fake_set_gateway_hid(const char *hid) {
    g_hid = hid;
}

void fake_set_device_hid(const char *hid) {
    d_hid = hid;
}

int restore_gateway_info(arrow_gateway_t *gateway) {
    if ( g_hid ) {
        property_t t = p_stack(g_hid);
      property_copy(&gateway->hid, t);
      return 0;
    }
    return -1;
}


void save_gateway_info(const arrow_gateway_t *gateway) {
    g_hid = gateway->hid.value;
}

int restore_device_info(arrow_device_t *device) {
  if ( !d_hid ) return -1;
  property_copy(&device->hid, p_stack(d_hid));
#if defined(__IBM__)
    property_copy(&device->eid, p_stack(dev_eid));
#endif
    return 0;
}

void save_device_info(arrow_device_t *dev) {
    d_hid = dev->hid.value;
}
