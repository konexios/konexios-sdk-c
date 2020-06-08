#ifndef ACN_SDK_C_PRIVATE_H_
#define ACN_SDK_C_PRIVATE_H_

#define DEFAULT_API_KEY             "5a86fccc83703142f1caa97ad38a01ad4985bb90956cfc7a3104342c961ab24c"
#define DEFAULT_SECRET_KEY          "2O2GrpT1x09tMddzXPMt5QLG8wB16I0I15ZbeA/6d00="

#define DEFAULT_WIFI_SSID           "yourSSID"
#define DEFAULT_WIFI_PASS           "123456789"
#define DEFAULT_WIFI_SEC            0x00040003


/* gateway */
#define GATEWAY_UID_PREFIX          "b-l475e-iot01a"
#define GATEWAY_NAME                "B-L475-gateway-demo"
#define GATEWAY_OS                  "FreeRTOS"

/* gateway firmware */
#define GATEWAY_SOFTWARE_NAME       "FreeRTOS-FW"
#define GATEWAY_SOFTWARE_VERSION    "1.0.1"

/* device */
#define DEVICE_NAME                 "b-l475e-iot01a"
#define DEVICE_TYPE                 "Local"
#define DEVICE_UID_SUFFIX           "b-l475e-iot01a-dev"


#define HTTP_CIPHER
#define MQTT_CIPHER
#define __arm__


#define STATIC_JSON
#define STATIC_DYNAMIC_PROPERTY
#define ARROW_MAX_JSON_OBJECTS           (16*10)
#define ARROW_JSON_STATIC_BUFFER_SIZE    (16*160)
#define ARROW_DYNAMIC_STATIC_BUFFER_SIZE (32*20)

#endif //
