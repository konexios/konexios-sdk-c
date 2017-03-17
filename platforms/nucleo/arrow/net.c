#include "NetworkSocketAPI/WiFi_c.h"
int get_mac_address(char *mac) {
    return wifi_mac_address(mac);
}
