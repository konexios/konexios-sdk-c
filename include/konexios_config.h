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

typedef enum {
    arrow_scheme_http,
    arrow_scheme_https
} scheme_http_t;

typedef enum {
    arrow_mqtt_scheme_tcp,
    arrow_mqtt_scheme_tls
} mqtt_scheme_t;


typedef struct {
    char mqtthost[255];
    mqtt_scheme_t mqtt_scheme;
    uint16_t mqttport;
    char     mqttvirtualhost[50];
} IoT_Client_Init_Mqtt;

#if defined(MQTT_CIPHER)
#define IoT_Client_Init_Mqtt_Default2             \
  ((IoT_Client_Init_Mqtt)                       \
     {                                             \
        .mqtthost = { MQTT_COMMAND_ADDR }         \
       ,.mqtt_scheme = arrow_mqtt_scheme_tls      \
       ,.mqttport    = 8883                       \
    })
#else    //MQTT_CIPHER
#define IoT_Client_Init_Mqtt_Default2             \
  ((IoT_Client_Init_Mqtt)                       \
    {                                             \
        .mqtthost = { MQTT_COMMAND_ADDR }         \
       ,.mqtt_scheme = arrow_mqtt_scheme_tcp      \
       ,.mqttport    = 1883                       \
    })
#endif   //MQTT_CIPHER
extern IoT_Client_Init_Mqtt iotClientInitMqttDefault;

typedef struct {
     char apikey[256];
     char secretkey[256];
}IoT_Client_Init_Key;

extern IoT_Client_Init_Key iotClientInitKeyDefault;

typedef struct {
     char  apihost[255];
     scheme_http_t api_scheme;
     uint16_t apiport;
}IoT_Client_Init_Api;

#if defined(HTTP_CIPHER)
#define iotClientInitApiDefault2 						      \
		((IoT_Client_Init_Api)   						          \
		{     											                  \
        	.apihost ={ ARROW_ADDR }      				  \
           ,.api_scheme  = arrow_scheme_https     \
           ,.apiport	 = 443                      \
		})
#else   // HTTP_CIPHER
		((IoT_Client_Init_Api)   						          \
		{     											                  \
			.apihost ={ ARROW_ADDR }                    \
		   ,.api_scheme  = arrow_scheme_http          \
		   ,.apiport	 = 12001                        \
		})
#endif  // HTTP_CIPHER

extern IoT_Client_Init_Api iotClientInitApiDefault;


#endif /* ACN_SDK_C_KONEXIOS_CONFIG_H_ */
