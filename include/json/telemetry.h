/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef KONEXIOS_TELEMETRY_H_
#define KONEXIOS_TELEMETRY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <konexios/device.h>
    
JsonNode *telemetry_serialize_json(konexios_device_t *device, void *data);
property_t telemetry_serialize(konexios_device_t *device, void *data);

#if defined(__cplusplus)
}
#endif

#endif /* KONEXIOS_TELEMETRY_H_ */
