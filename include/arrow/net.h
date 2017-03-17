#ifndef KRONOS_C_SDK_ARROW_NET_H_
#define KRONOS_C_SDK_ARROW_NET_H_

int get_mac_address(char *mac);

#if defined(_ARIS_)
# include "driver/source/nmasic.h"
# if defined(ETH_MODE)
#  include "nxd_bsd.h"
# endif
#elif defined(__MBED__)
//# include "WiFi.h"
//# include <type>
#elif defined(__linux__)
# include <stdlib.h>
#endif

#endif  // KRONOS_C_SDK_ARROW_NET_H_
