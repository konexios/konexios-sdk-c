/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_TELEMETRY_H_
#define ARROW_TELEMETRY_H_

#include <arrow/device.h>
#if defined(_ARIS_)
#include "sensors_data.h"
#else
typedef void sensor_data_t;
#endif
#if defined(__linux__)
typedef struct {
  float temperature_core0;
  float temperature_core1;
} probook_data_t;
#elif defined(__XCC__)
# include <qcom_common.h>
typedef struct {
   A_UINT8 rssi;
   float temperature;
} rssi_data_t;
#endif


char *telemetry_serialize(arrow_device_t *device, sensor_data_t *data);

#endif /* ARROW_TELEMETRY_H_ */
