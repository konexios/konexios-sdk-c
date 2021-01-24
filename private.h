#ifndef KONEXIOS_SDK_C_PRIVATE_H_
#define KONEXIOS_SDK_C_PRIVATE_H_

#define DEFAULT_API_KEY             "your-api-key"
#define DEFAULT_SECRET_KEY          "your-secret-key"

#define KONEXIOS_ADDR               "api-helios.konexios.io"

#define MQTT_COMMAND_ADDR           "mqtt-helios.konexios.io"
#define VHOST "/pegasus"

#define DEFAULT_WIFI_SSID           "your-wifi-ssid"
#define DEFAULT_WIFI_PASS           "your-wifi-password"
#define DEFAULT_WIFI_SEC            0x00040003

/* gateway */
#define GATEWAY_UID_PREFIX          "your-gateway-uid-prefix"
#define GATEWAY_NAME                "your-gateway-name"
#define GATEWAY_OS                  "your-gateway-os"

/* gateway firmware */
#define GATEWAY_SOFTWARE_NAME       "your-gateway-software-name"
#define GATEWAY_SOFTWARE_VERSION    "1.0.0"

/* device */
#define DEVICE_NAME                 "your-device-name"
#define DEVICE_TYPE                 "Local"
#define DEVICE_UID_SUFFIX           "your-device-uid-suffix"

#define HTTP_CIPHER
#define MQTT_CIPHER
#define __arm__

#define STATIC_JSON
#define STATIC_DYNAMIC_PROPERTY
#define KONEXIOS_MAX_JSON_OBJECTS           (16*10)
#define KONEXIOS_JSON_STATIC_BUFFER_SIZE    (16*160)
#define KONEXIOS_DYNAMIC_STATIC_BUFFER_SIZE (32*20)

#endif //
