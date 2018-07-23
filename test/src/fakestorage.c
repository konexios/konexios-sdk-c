#include "fakestorage.h"
#include <string.h>

static char d_hid[64] = {0};
static char g_hid[64] = {0};

void fake_set_gateway_hid(const char *hid) {
    if ( !hid ) memset(g_hid, 0x0, sizeof(g_hid));
    else strcpy(g_hid, hid);
}

void fake_set_device_hid(const char *hid) {
    if ( !hid ) memset(d_hid, 0x0, sizeof(d_hid));
    else strcpy(d_hid, hid);
}

int restore_gateway_info(arrow_gateway_t *gateway) {
    if ( g_hid[0] ) {
        property_t t = p_stack(g_hid);
      property_copy(&gateway->hid, t);
      return 0;
    }
    return -1;
}


void save_gateway_info(const arrow_gateway_t *gateway) {
    strcpy(g_hid, gateway->hid.value);
}

int restore_device_info(arrow_device_t *device) {
  if ( !d_hid[0] ) return -1;
  property_copy(&device->hid, p_stack(d_hid));
#if defined(__IBM__)
    property_copy(&device->eid, p_stack(dev_eid));
#endif
    return 0;
}

void save_device_info(arrow_device_t *dev) {
    strcpy(d_hid, dev->hid.value);
}
