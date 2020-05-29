/* Copyright (c) 2020 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_KONEXIOS_CONFIG_H_
#define ACN_SDK_C_KONEXIOS_CONFIG_H_

#include <stdint.h>
#include "private.h"

#define xstr(s) str(s)
#define str(s) #s

#define SDK_VERSION 1.3.12

#if !defined(RINGBUFFER_SIZE)
#define RINGBUFFER_SIZE		    1024
#endif

#if defined(STATIC_ACN)
#define STATIC_MQTT_ENV
#define STATIC_HTTP_CLIENT
#define STATIC_SIGN
# if !defined(ARROW_MAX_MQTT_COMMANDS)
# define ARROW_MAX_MQTT_COMMANDS 10
# endif
#endif

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(__MIPSEL__) || \
    defined(__XTENSA_EL__)
#define __LE_MODE__ 1
#elif (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(__MIPSEB__) || \
    defined(__XTENSA_EB__)
#define __BE_MODE__ 1
#else
#warning "Undefined endian mode __LE_MODE__/__BE_MODE__ [little by default]"
#endif


#if !defined(__NO_STD__)
# define __USE_STD__
// for the std lib headers
# if !defined(_GNU_SOURCE)
#  define _GNU_SOURCE
# endif
#endif

#if !defined(SSP_PARAMETER_NOT_USED)
# define SSP_PARAMETER_NOT_USED(x) (void)((x))
#endif

#if defined(STATIC_HTTP_CLIENT)
# define RING_BUFFER_ARRAY
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
#if !defined(ARROW_MAX_RETRY)
#define ARROW_MAX_RETRY -1
#endif
#define RETRY_UP(r, overact) \
    if ( r >= 0 && ++r > ARROW_MAX_RETRY ) { overact; }
#define RETRY_CR(r) \
    r = 0;
#define TO_FAHRENHEIT(x) ((x)*1.8 + 32)

#if !defined(__attribute_warn_unused_result__)
#define __attribute_warn_unused_result__ __attribute__ ((warn_unused_result))
#endif

/* NTP server settings */
#define NTP_DEFAULT_SERVER  "0.pool.ntp.org"
#define NTP_DEFAULT_PORT    123
#define NTP_DEFAULT_TIMEOUT 4000
/***********************************************************************/
/*                  Define for restful API                             */
/***********************************************************************/
#if !defined(DEFAULT_API_TIMEOUT)
# define DEFAULT_API_TIMEOUT 10000
#endif


#define ARROW_API_BASE_URL                  ARROW_SCH "://" ARROW_ADDR ":" xstr(ARROW_PORT)
#define ARROW_API_GATEWAY_ENDPOINT          "/api/v1/kronos/gateways"
#define ARROW_API_DEVICE_ENDPOINT           "/api/v1/kronos/devices"
#define ARROW_API_TELEMETRY_ENDPOINT        "/api/v1/kronos/telemetries"
#define ARROW_API_EVENTS_ENDPOINT           "/api/v1/core/events"
#define ARROW_API_ACCOUNT_ENDPOINT          "/api/v1/kronos/accounts"
#define ARROW_API_NODE_ENDPOINT             "/api/v1/kronos/nodes"
#define ARROW_API_NODE_TYPE_ENDPOINT        "/api/v1/kronos/nodes/types"
#define ARROW_API_TESTSUITE_ENDPOINT        "/api/v1/kronos/testsuite"
#define ARROW_API_SOFTWARE_RELEASE_ENDPOINT "/api/v1/kronos/software/releases/transactions"
#define ARROW_API_SOFTWARE_RELEASE_SCHEDULE_ENDPOINT "/api/v1/kronos/software/releases/schedules"

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

#if !defined(DEVICE_SOFTWARE_NAME)
#define DEVICE_SOFTWARE_NAME GATEWAY_SOFTWARE_NAME
#endif
#if !defined(DEVICE_SOFTWARE_VERSION)
#define DEVICE_SOFTWARE_VERSION GATEWAY_SOFTWARE_VERSION
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

#if !defined(ARROW_ERROR_DELAY)
#define ARROW_RETRY_DELAY 3000
#endif
#if !defined(MQTT_ERROR_DELAY)
#define MQTT_RETRY_DELAY 6000
#endif

/***********************************************************************/
/*                  Define for MQTT                                    */
/***********************************************************************/

#if !defined(DEFAULT_MQTT_TIMEOUT)
# define DEFAULT_MQTT_TIMEOUT 10000
#endif

#if defined(__IBM__) || defined(__AZURE__)
#define MQTT_TWO_CHANNEL
# define MQTT_CIPHER
#endif

#if !defined(MQTT_BUF_LEN)
#define MQTT_BUF_LEN 400
#endif

#if !defined(MQTT_RECVBUF_LEN)
#define MQTT_RECVBUF_LEN 1200
#endif

#if defined(ARROW_THREAD) &&  \
    !defined(__IBM__) && \
    !defined(__AZURE__)
# warning "There is only one possible MQTT connection"
# undef ARROW_THREAD
#endif

#if !defined(S_TOP_NAME)
#define S_TOP_NAME "krs/cmd/stg/"
#endif
#if !defined(P_TOP_NAME)
#define P_TOP_NAME "krs.tel.gts."
#endif

#if !defined(PX_TOP_NAME)
#define PX_TOP_NAME "krs.api.gts."
#endif
#if !defined(SX_TOP_NAME)
#define SX_TOP_NAME "krs/api/stg/"
#endif

#if defined(__IBM__)
#  define MQTT_TELEMETRY_ADDR ".messaging.internetofthings.ibmcloud.com"
#elif defined(__AZURE__)
#  define MQTT_TELEMETRY_ADDR "pgshubdev01.azure-devices.net"
#  define VHOST "iothubowner"
#else
# if !defined(MQTT_TELEMETRY_ADDR)
#  define MQTT_TELEMETRY_ADDR MQTT_COMMAND_ADDR
# endif
#endif

#define ARROW_MQTT_URL MQTT_SCH "://" MQTT_ADDR ":" #MQTT_PORT

#if !defined(MQTT_QOS)
#define MQTT_QOS        1
#endif
#if !defined(MQTT_RETAINED)
#define MQTT_RETAINED   0
#endif
#if !defined(MQTT_ORIGINAL)
#define MQTT_ORIGINAL        0
#endif

typedef enum {
    arrow_scheme_http,
    arrow_scheme_https
} scheme_http_t;

typedef enum {
    arrow_mqtt_scheme_tcp,
    arrow_mqtt_scheme_tls
} mqtt_scheme_t;


typedef struct {
    char host[255];
    mqtt_scheme_t scheme;
    uint16_t port;
    char     virtualhost[50];
} IoT_Client_Init_Mqtt;

#if defined(MQTT_CIPHER)
#define IOT_CLIENT_INIT_MQTT_DEFAULT              \
  ((IoT_Client_Init_Mqtt)                         \
     {                                            \
        .host = { MQTT_COMMAND_ADDR }             \
       ,.scheme = arrow_mqtt_scheme_tls           \
       ,.port    = 8883                           \
	   ,.virtualhost = { VHOST }                  \
    })
#else    //MQTT_CIPHER
#define IOT_CLIENT_INIT_MQTT_DEFAULT              \
  ((IoT_Client_Init_Mqtt)                         \
    {                                             \
        .host = { MQTT_COMMAND_ADDR }             \
       ,.scheme = arrow_mqtt_scheme_tcp           \
       ,.port    = 1883                           \
       ,.virtualhost = { VHOST }                  \
    })
#endif   //MQTT_CIPHER
extern IoT_Client_Init_Mqtt iotClientInitMqtt;

typedef struct {
     char apikey[256];
     char secretkey[256];
}IoT_Client_Init_Key;

extern IoT_Client_Init_Key iotClientInitKey;

typedef struct {
     char  host[255];
     scheme_http_t scheme;
     uint16_t port;
}IoT_Client_Init_Api;

#if defined(HTTP_CIPHER)
#define IOT_CLIENT_INIT_API_DEFAULT               \
	((IoT_Client_Init_Api)                        \
      {                                           \
            .host ={ ARROW_ADDR }                 \
           ,.scheme  = arrow_scheme_https         \
           ,.port	 = 443                        \
        })
#else   // HTTP_CIPHER
#define IOT_CLIENT_INIT_API_DEFAULT               \
     ((IoT_Client_Init_Api)                       \
      {                                           \
            .host ={ ARROW_ADDR }                 \
           ,.scheme  = arrow_scheme_http          \
           ,.port	 = 12001                      \
        })
#endif  // HTTP_CIPHER

extern IoT_Client_Init_Api iotClientInitApi;


#endif /* ACN_SDK_C_KONEXIOS_CONFIG_H_ */
