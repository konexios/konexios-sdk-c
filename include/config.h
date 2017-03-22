/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define xstr(s) str(s)
#define str(s) #s

//#define __IBM__
//#define __AZURE__
//#define DEV_ENV
//#define DEBUG

#define SDK_VERSION 1.1.0

#if defined(__IBM__)
//#define HTTP_CIPHER
//#define MQTT_CIPHER
#elif defined(__AZURE__)
//  #define HTTP_CIPHER
#  define MQTT_CIPHER
#else // for IoT
#  if !defined(DEV_ENV)
#   define HTTP_CIPHER
#  endif
//#define MQTT_CIPHER
#endif

#if defined(__linux__) \
 || defined( _ARIS_)    \
 || defined(__MBED__)  \
 || defined(__XCC__)
#else
# error "platform doesn't support"
#endif


#if defined(__XCC__)
# define __NO_STD__
#else
# define __USE_STD__
#endif

#if !defined(SSP_PARAMETER_NOT_USED)
# define SSP_PARAMETER_NOT_USED(x) (void)((x))
#endif

#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000

/* Initialize AP mode parameters structure with SSID, channel and OPEN security type. */
#if defined(_ARIS_)
# define MAIN_WLAN_SSID      "ARIS_WIFI"
#elif defined(__MBED__)
# define MAIN_WLAN_SSID      "NUCLEO_WIFI"
#elif defined(__XCC__)
# define MAIN_WLAN_SSID      "QCA4010_WIFI"
#endif

#define MAIN_WLAN_CHANNEL   5
#define MAIN_WLAN_AUTH      1 //M2M_WIFI_SEC_OPEN

/* cloud connectivity */
#if defined(HTTP_CIPHER)
# define ARROW_SCH "https"
# define ARROW_PORT 443
#else
# define ARROW_SCH "http"
# define ARROW_PORT 12001
#endif
#if defined(DEV_ENV)
# define ARROW_ADDR "pgsdev01.arrowconnect.io"
#else
# define ARROW_ADDR "api.arrowconnect.io"
#endif

#if defined(MQTT_CIPHER)
#  define MQTT_SCH "tls"
# define MQTT_PORT 8883
#else
# define MQTT_SCH "tcp"
# define MQTT_PORT 1883
#endif

#if defined(__IBM__)
#  define MQTT_ADDR ".messaging.internetofthings.ibmcloud.com"
#elif defined(__AZURE__)
#  define MQTT_ADDR "pgshubdev01.azure-devices.net"
#  define VHOST "iothubowner"
#else
# if defined(DEV_ENV)
#  define MQTT_ADDR "pgsdev01.arrowconnect.io"
#  define VHOST "/themis.dev:"
# else
#  define MQTT_ADDR "mqtt-a01.arrowconnect.io"
#  define VHOST "/pegasus:"
# endif
#endif

#define ARROW_API_BASE_URL              ARROW_SCH"://"ARROW_ADDR":" xstr(ARROW_PORT)
#define ARROW_API_GATEWAY_ENDPOINT      ARROW_API_BASE_URL"/api/v1/kronos/gateways"
#define ARROW_API_DEVICE_ENDPOINT       ARROW_API_BASE_URL"/api/v1/kronos/devices"
#define ARROW_API_TELEMETRY_ENDPOINT    ARROW_API_BASE_URL"/api/v1/kronos/telemetries"
#define ARROW_MQTT_URL                  MQTT_SCH"://"MQTT_ADDR":" xstr(MQTT_PORT)

/* gateway configuration */

#if defined(_ARIS_)
#define GATEWAY_UID_PREFIX          "aris"
#define GATEWAY_NAME                "aris-gateway-demo"
#define GATEWAY_OS                  "ThreadX"
#elif defined(__MBED__) //TARGET_NUCLEO_F401RE
#define GATEWAY_UID_PREFIX          "nucleo"
#define GATEWAY_NAME                "my-test-gateway-123"
#define GATEWAY_OS                  "mbed"
#elif defined(__linux__)
#define GATEWAY_UID_PREFIX          "probook"
#define GATEWAY_NAME                "probook-gateway-demo"
#define GATEWAY_OS                  "linux"
#elif defined(__XCC__)
#define GATEWAY_UID_PREFIX          "QCA"
#define GATEWAY_NAME                "QCA-gateway-demo"
#define GATEWAY_OS                  "ThreadX"
#else
# error "Not supported platform"
#endif
#define GATEWAY_TYPE                "Local"
#define GATEWAY_SOFTWARE_NAME       "eos"
#define GATEWAY_SOFTWARE_VERSION    "0.1"

/* device configuration */

#if defined(_ARIS_)
#define DEVICE_NAME         "aris-device-demo"
#define DEVICE_TYPE         "aris-device"
#define DEVICE_UID_SUFFIX   "board"
#elif defined(TARGET_NUCLEO_F401RE)
# if defined(SENSOR_TILE)
#  define DEVICE_UID_SUFFIX   "sensortile"
#  define DEVICE_NAME         "nucleo-sensortile"
#  define DEVICE_TYPE         "st-sensortile"
# else
#  define DEVICE_UID_SUFFIX   "iks01a1"
#  define DEVICE_NAME         "nucleo iks01a1"
#  define DEVICE_TYPE         "x-nucleo-iks01a1"
# endif
#elif defined(__linux__)
#define DEVICE_NAME         "probook-4540s"
#define DEVICE_TYPE         "hp-probook-4540s"
#define DEVICE_UID_SUFFIX   "notebook"
#elif defined(__XCC__)
#define DEVICE_NAME         "ULPGN"
#define DEVICE_TYPE         "SX_ULPGN"
#define DEVICE_UID_SUFFIX   "devkit"
#else
# error "Not supported platform"
#endif

/* telemetry configuration */

#define TELEMETRY_DEVICE_HID        "_|deviceHid"
#define TELEMETRY_TEMPERATURE       "f|temperature"
#define TELEMETRY_HUMIDITY          "f|humidity"
#define TELEMETRY_BAROMETER         "f|barometer"
#define TELEMETRY_ACCELEROMETER_X   "f|accelerometerX"
#define TELEMETRY_ACCELEROMETER_Y   "f|accelerometerY"
#define TELEMETRY_ACCELEROMETER_Z   "f|accelerometerZ"
#define TELEMETRY_GYROMETER_X       "f|gyrometerX"
#define TELEMETRY_GYROMETER_Y       "f|gyrometerY"
#define TELEMETRY_GYROMETER_Z       "f|gyrometerZ"
#define TELEMETRY_MAGNETOMETER_X    "f|magnetometerX"
#define TELEMETRY_MAGNETOMETER_Y    "f|magnetometerY"
#define TELEMETRY_MAGNETOMETER_Z    "f|magnetometerZ"
#define TELEMETRY_DELAY             5000

#if !defined(DEFAULT_API_KEY)
#error "set the API key"
#endif
#if !defined(DEFAULT_SECRET_KEY)
#error "set the SECRET key"
#endif

#endif /* CONFIG_H_ */
