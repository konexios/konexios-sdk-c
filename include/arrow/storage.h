#ifndef ARROW_STORAGE_H_
#define ARROW_STORAGE_H_

#include <arrow/device.h>
#include <arrow/gateway.h>

int restore_gateway_info(arrow_gateway_t *gateway);
void save_gateway_info(const arrow_gateway_t *gateway);

int restore_device_info(arrow_device_t *device);
void save_device_info(arrow_device_t *device);

void save_wifi_setting(const char *ssid, const char *pass, int sec);
int restore_wifi_setting(char *ssid, char *pass, int *sec);

#endif
