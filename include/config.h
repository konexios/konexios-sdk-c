/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef SDK_C_CONFIG_H_
#define SDK_C_CONFIG_H_

#define xstr(s) str(s)
#define str(s) #s

#define SDK_VERSION 1.3.6

#if !defined(_KEYS_)
#include "private.h"
#endif

#if !defined(DEFAULT_MQTT_TIMEOUT)
# define DEFAULT_MQTT_TIMEOUT 10000
#endif
#if !defined(DEFAULT_API_TIMEOUT)
# define DEFAULT_API_TIMEOUT 10000
#endif

#if defined(__IBM__)
//#define HTTP_CIPHER
//#define MQTT_CIPHER
#elif defined(__AZURE__)
//  #define HTTP_CIPHER
#  define MQTT_CIPHER
#else // for IoT
#  if !defined(DEV_ENV)
#   define HTTP_CIPHER
#   define MQTT_CIPHER
#  endif
//#  define MQTT_CIPHER
#endif

#if !defined(__NO_STD__)
# if !defined(__XCC__)
#  define __USE_STD__
// for the std lib headers
#  if !defined(_GNU_SOURCE)
#   define _GNU_SOURCE
#  endif
# endif
#endif

#if !defined(SSP_PARAMETER_NOT_USED)
# define SSP_PARAMETER_NOT_USED(x) (void)((x))
#endif

/* NTP server settings */
#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#define NTP_DEFAULT_PORT 123
#define NTP_DEFAULT_TIMEOUT 4000

/* Initialize AP mode parameters structure with SSID, channel and OPEN security type. */
#if !defined(MAIN_WLAN_SSID)
#  define MAIN_WLAN_SSID      "ACN_WIFI"
#endif

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
#  define MQTT_PORT 8883
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

#define ARROW_API_BASE_URL                  ARROW_SCH "://" ARROW_ADDR ":" xstr(ARROW_PORT)
#define ARROW_API_GATEWAY_ENDPOINT          ARROW_API_BASE_URL "/api/v1/kronos/gateways"
#define ARROW_API_DEVICE_ENDPOINT           ARROW_API_BASE_URL "/api/v1/kronos/devices"
#define ARROW_API_TELEMETRY_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/telemetries"
#define ARROW_API_EVENTS_ENDPOINT           ARROW_API_BASE_URL "/api/v1/core/events"
#define ARROW_API_ACCOUNT_ENDPOINT          ARROW_API_BASE_URL "/api/v1/kronos/accounts"
#define ARROW_API_NODE_ENDPOINT             ARROW_API_BASE_URL "/api/v1/kronos/nodes"
#define ARROW_API_NODE_TYPE_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/nodes/types"
#define ARROW_API_TESTSUITE_ENDPOINT        ARROW_API_BASE_URL "/api/v1/kronos/testsuite"
#define ARROW_API_SOFTWARE_RELEASE_ENDPOINT ARROW_API_BASE_URL "/api/v1/kronos/software/releases/transactions"
#define ARROW_MQTT_URL                      MQTT_SCH "://" MQTT_ADDR ":" xstr(MQTT_PORT)

/* default gateway and device configuration */
/* default gateway configuration */
#if !defined(GATEWAY_UID_PREFIX)
# define GATEWAY_UID_PREFIX          "unknown"
#endif
#if !defined(GATEWAY_NAME)
#  define GATEWAY_NAME                GATEWAY_UID_PREFIX "-gateway"
#endif
#if !defined(GATEWAY_OS)
# define GATEWAY_OS                  "none"
#endif

#if !defined(GATEWAY_TYPE)
#define GATEWAY_TYPE                "Local"
#endif
#if !defined(GATEWAY_SOFTWARE_NAME)
#define GATEWAY_SOFTWARE_NAME       "eos"
#endif
#if !defined(GATEWAY_SOFTWARE_VERSION)
#define GATEWAY_SOFTWARE_VERSION    "0.1"
#endif

/* device configuration */
#if !defined(DEVICE_NAME)
#define DEVICE_NAME                 "unknown"
#endif
#if !defined(DEVICE_TYPE)
#define DEVICE_TYPE                 "unknown"
#endif
#if !defined(DEVICE_UID_SUFFIX)
#define DEVICE_UID_SUFFIX           "dev"
#endif

/* telemetry configuration */
#define TELEMETRY_DEVICE_HID        "_|deviceHid"
#define TELEMETRY_TEMPERATURE       "f|temperature"
#define TELEMETRY_HUMIDITY          "f|humidity"
#define TELEMETRY_BAROMETER         "f|barometer"
#define TELEMETRY_ACCELEROMETER_XYZ "f3|accelerometerXYZ"
#define TELEMETRY_ACCELEROMETER_X   "f|accelerometerX"
#define TELEMETRY_ACCELEROMETER_Y   "f|accelerometerY"
#define TELEMETRY_ACCELEROMETER_Z   "f|accelerometerZ"
#define TELEMETRY_GYROMETER_XYZ     "f3|gyrometerXYZ"
#define TELEMETRY_GYROMETER_X       "f|gyrometerX"
#define TELEMETRY_GYROMETER_Y       "f|gyrometerY"
#define TELEMETRY_GYROMETER_Z       "f|gyrometerZ"
#define TELEMETRY_MAGNETOMETER_XYZ  "f3|magnetometerXYZ"
#define TELEMETRY_MAGNETOMETER_X    "f|magnetometerX"
#define TELEMETRY_MAGNETOMETER_Y    "f|magnetometerY"
#define TELEMETRY_MAGNETOMETER_Z    "f|magnetometerZ"
#if !defined(TELEMETRY_DELAY)
#define TELEMETRY_DELAY             5000
#endif
#define TO_FAHRENHEIT(x) ((x)*1.8 + 32)

#if !defined(ARROW_ERROR_DELAY)
#define ARROW_RETRY_DELAY 3000
#endif


#endif /* SDK_C_CONFIG_H_ */
