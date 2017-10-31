#include <stdio.h>
#include "config.h"

int main() {
  printf("gateway uid:\t%s-<mac>\r\n", GATEWAY_UID_PREFIX);
  printf("gateway name:\t%s\r\n", GATEWAY_NAME);
  printf("gateway os:\t%s\r\n", GATEWAY_OS);
  printf("gateway type:\t%s\r\n", GATEWAY_TYPE);
  printf("device name:\t%s\r\n", DEVICE_NAME);
  printf("device type:\t%s\r\n", DEVICE_TYPE);
  printf("device uid:\t%s-<mac>-%s\r\n", GATEWAY_UID_PREFIX, DEVICE_UID_SUFFIX);
  return 0;
}
