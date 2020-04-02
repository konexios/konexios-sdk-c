#ifndef ACN_SDK_C_PRIVATE_H_
#define ACN_SDK_C_PRIVATE_H_

#define DEFAULT_API_KEY             "abc"
#define DEFAULT_SECRET_KEY          "xyz"

#define DEFAULT_WIFI_SSID           "yourSSID"
#define DEFAULT_WIFI_PASS           "password"
#define DEFAULT_WIFI_SEC            0x00040003

/* gateway */
#define GATEWAY_UID_PREFIX          "QCA"
#define GATEWAY_NAME                "QCA-gateway-demo"
#define GATEWAY_OS                  "Linux"

/* gateway firmware */
#define GATEWAY_SOFTWARE_NAME       "SX-ULPGN-EVK-TEST-FW"
#define GATEWAY_SOFTWARE_VERSION    "1.2.4"

/* device */
#define DEVICE_NAME                 "ULPGN"
#define DEVICE_TYPE                 "SX_ULPGN"
#define DEVICE_UID_SUFFIX           "devkit"

#define ARROW_MAX_JSON_OBJECTS           10
#define ARROW_JSON_STATIC_BUFFER_SIZE   992 
#define ARROW_DYNAMIC_STATIC_BUFFER_SIZE 1
#define STATIC_JSON
#define STATIC_DYNAMIC_PROPERTY

# define HTTP_CIPHER

#define __arm__

#endif //
