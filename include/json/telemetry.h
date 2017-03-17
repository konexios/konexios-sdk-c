/*
 * telemetry.h
 *
 *  Created on: 29 окт. 2016 г.
 *      Author: ddemidov
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
