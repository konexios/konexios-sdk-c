#include <qcom_common.h>
#include <qcom_misc.h>
A_UINT8 __currentDeviceId = 0;

int get_mac_address(char *mac) {
  return qcom_mac_get(__currentDeviceId, (A_UINT8*)mac);
}

