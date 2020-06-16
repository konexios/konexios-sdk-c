/* Copyright (c) 2020 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */
#include "konexios_config.h"

IoT_Client_Init_Mqtt iotClientInitMqtt = IOT_CLIENT_INIT_MQTT_DEFAULT;


IoT_Client_Init_Key iotClientInitKey   ={
		.apikey = { DEFAULT_API_KEY }
	   ,.secretkey = { DEFAULT_SECRET_KEY }
	};


IoT_Client_Init_Api iotClientInitApi =  IOT_CLIENT_INIT_API_DEFAULT;


