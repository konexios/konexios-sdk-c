#include "arrow/net.h"

int get_mac_address(char *mac) {
  return nmi_get_mac_address(mac);
}
