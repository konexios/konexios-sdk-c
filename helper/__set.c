#if !defined(_KEYS_)
#include "private.h"
#endif
#include <stdio.h>

#if !defined(DEFAULT_API_KEY)
#warning "There is no DEFAULT_API_KEY key into acn-sdk-c/private.h file"
#endif

#if !defined(DEFAULT_SECRET_KEY)
#warning "There is no DEFAULT_SECRET_KEY key into acn-sdk-c/private.h file"
#endif

#if defined(__linux__) \
    || defined(_ARIS_) \
    || defined(__nucleo__) \
    || defined(__silex__) \
    || defined(__senseability__) \
    || defined(__stm32l475iot__) \
    || defined(__semiconductor__) \
    || defined(__quadro__)
#else
# warning "probably this platform doesn't support"
#endif

#if !defined(GATEWAY_UID_PREFIX)
# warning "There is no GATEWAY_UID_PREFIX value into acn-sdk-c/private.h file; use default value"
#endif

#if !defined(GATEWAY_OS)
# warning "There is no GATEWAY_OS value into acn-sdk-c/private.h file; use default value"
#endif

#if !defined(GATEWAY_TYPE)
#pragma message ("There is no GATEWAY_TYPE value into acn-sdk-c/private.h file; use default value")
#endif

#if !defined(GATEWAY_SOFTWARE_NAME)
#warning "There is no GATEWAY_SOFTWARE_NAME value into acn-sdk-c/private.h file; use default value"
#endif

#if !defined(GATEWAY_SOFTWARE_VERSION)
#warning "There is no GATEWAY_SOFTWARE_VERSION value into acn-sdk-c/private.h file; use default value"
#endif

#if !defined(DEVICE_NAME)
#warning "There is no DEVICE_NAME value into acn-sdk-c/private.h file; use \"unknown\" by default"
#endif
#if !defined(DEVICE_TYPE)
#warning "There is no DEVICE_TYPE value into acn-sdk-c/private.h file; use \"unknown\" by default"
#endif
#if !defined(DEVICE_UID_SUFFIX)
#warning "There is no DEVICE_UID_SUFFIX value into acn-sdk-c/private.h file; use \"dev\" by default"
#endif


int main() {
  printf("Settings OK\r\n");
  return 0;
}
