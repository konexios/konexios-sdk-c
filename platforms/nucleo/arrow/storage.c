#include "arrow/storage.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <arrow/utf8.h>
#include "flashmbed.h"

typedef struct {
  int magic;
  char ssid[64];
  char pass[64];
  int sec;
  char padding[120];
  char gateway_hid[64];
  char device_hid[64];
  char device_eid[64];
  char unused[64];
} flash_mem_t;

int check_mgc() {
  int *c = flash_start();
  if ( *c != FLASH_MAGIC_NUMBER ) {
    return 0;
  }
  return 1;
}

int restore_gateway_info(arrow_gateway_t *gateway) {
  printf("restore gateway info\r\n");
  if (check_mgc()) {
    flash_mem_t *mem = (flash_mem_t *)flash_start();
    if ( utf8check(mem->gateway_hid) && strlen(mem->gateway_hid) > 0 ) {
      arrow_gateway_add_hid(gateway, mem->gateway_hid);
      return 0;
    }
  }
  return -1;
}

void save_gateway_info(const arrow_gateway_t *gateway) {
  printf("new registration\r\n");
  flash_mem_t mem;
  memcpy(&mem, flash_start(), sizeof(flash_mem_t));
  strcpy(mem.gateway_hid, gateway->hid);
  write_flash(&mem, sizeof(flash_mem_t));
}

int restore_device_info(arrow_device_t *device) {
  if (check_mgc()) {
    flash_mem_t *mem = (flash_mem_t *)flash_start();
    if ( !utf8check(mem->device_hid) || strlen(mem->device_hid) == 0 ) {
      return -1;
    }
    arrow_device_set_hid(device, mem->device_hid);
  #if defined(__IBM__)
    if ( !utf8check(mem->device_eid) || strlen(mem->device_eid) == 0 ) {
      return -1;
    }
    arrow_device_set_eid(device, mem->device_eid);
  #endif
    return 0;
  }
  return -1;
}

void save_device_info(arrow_device_t *device) {
  flash_mem_t mem;
  memcpy(&mem, flash_start(), sizeof(flash_mem_t));
  strcpy(mem.device_hid, device->hid);
#if defined(__IBM__)
  strcpy(mem.device_eid, device->eid);
#endif
  write_flash(&mem, sizeof(flash_mem_t));
}

void save_wifi_setting(const char *ssid, const char *pass, int sec) {
  flash_mem_t mem;
  memcpy(&mem, flash_start(), sizeof(flash_mem_t));
  strcpy(mem.ssid, ssid);
  strcpy(mem.pass, pass);
  mem.sec = sec;
  write_flash(&mem, sizeof(flash_mem_t));
}

int restore_wifi_setting(char *ssid, char *pass, int *sec) {
  if (check_mgc()) {
    flash_mem_t *mem = (flash_mem_t *)flash_start();
    strcpy(ssid, mem->ssid);
    strcpy(pass, mem->pass);
    *sec = mem->sec;
    return 0;
  }
  return -1;
}
