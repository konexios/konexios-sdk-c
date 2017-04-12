/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ARROW_TELEMETRY_H_
#define ARROW_TELEMETRY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <arrow/device.h>
#include <unint.h>
    
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

#elif defined(__senseability__)
typedef struct {
    uint8   Status;             /* Bit 0 - Humidicon Sensor Present
                                   Bit 1 - Airflow Sensor Present
                                   Bit 2 - Pressure Sensor Present
                                   Bit 3 - Hall Effect Sensor Present (Always = 1)
                                   Bit 4 - Temperature Alarm Low 
                                   Bit 5 - Temperature Alarm High
                                   Bit 6 - Reserved
                                   Bit 7 - Reserved                     */
    
 	int8	Temperature;	    /* in degress C */	
	uint8	Humidity;           /* Relative Humidity 00.0% to 100.0% */ 
    uint8   Magnetic_Switch;    /* Magnet Present = 0, No Magnet Present = 1 */ 
    int16	Pressure;           /* 0.0 to 1000.0 millbars in 10ths of millibars */
   	int16	Air_Flow;           /* +/- 200 SCCM */  
} Sensor_Data_t;

#define HUMIDICON_PRESENT_MASK  0x01
#define AIRFLOW_PRESENT_MASK    0x02
#define PRESSURE_PRESENT_MASK   0x04
#define MAGNET_PRESENT_MASK     0x08
#define TEMP_ALARM_LOW_MASK     0x10
#define TEMP_ALARM_HIGH_MASK    0x20

#elif defined(__semiconductor__)
typedef struct {
   int als;
   float abmienceInLux;
} gevk_data_t;
#endif

char *telemetry_serialize(arrow_device_t *device, sensor_data_t *data);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_TELEMETRY_H_ */
