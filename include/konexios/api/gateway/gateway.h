/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#if !defined(KONEXIOS_GATEWAY_API_H_)
#define KONEXIOS_GATEWAY_API_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <konexios/api/gateway/info.h>
#include <konexios/api/device/info.h>
#include <konexios/api/log.h>
#include <data/find_by.h>
#include <konexios/gateway.h>
#include <konexios/device.h>
#include <time/time.h>

typedef enum {
    GATEWAY_SUCCESS         = 0,
    GATEWAY_ERROR           = -100,
    GATEWAY_REGISTER_ERROR  = -101,
    GATEWAY_HEARTBEAT_ERROR = -102,
    GATEWAY_CHECKIN_ERROR   = -103,
    GATEWAY_CONFIG_ERROR    = -104,
    GATEWAY_FIND_ERROR      = -105,
    GATEWAY_FINDBY_ERROR    = -106,
    GATEWAY_LOGS_ERROR      = -107,
    GATEWAY_DEVLIST_ERROR   = -108,
    GATEWAY_DEVCOMS_ERROR   = -109,
    GATEWAY_UPDATE_ERROR    = -110
} konexios_gateway_error_t;

// register new gateway
int konexios_register_gateway(konexios_gateway_t *gateway);
// download gateway configuration
int konexios_gateway_config(konexios_gateway_t *gateway, konexios_gateway_config_t *config);
// send the heartbeat request
int konexios_gateway_heartbeat(konexios_gateway_t *gateway);
// send the checkin request
int konexios_gateway_checkin(konexios_gateway_t *gateway);
// find gateway by hid
int konexios_gateway_find(gateway_info_t *info, const char *hid);
// find gateway by other any parameters
int konexios_gateway_find_by(gateway_info_t **info, int n, ...);
// list gateway audit logs
int konexios_gateway_logs_list(log_t **logs, konexios_gateway_t *gateway, int n, ...);
// list gateway devices
int konexios_gateway_devices_list(device_info_t **list, const char *hid);
// send command and payload to gateway and device
int konexios_gateway_device_send_command(const char *gHid, const char *dHid, const char *cmd, const char *payload);
// update existing gateway
int konexios_gateway_update(konexios_gateway_t *gateway);
// send the error request
int konexios_gateway_error(konexios_gateway_t *gateway, const char *error);

#if defined(__cplusplus)
}
#endif

#endif  // KONEXIOS_GATEWAY_API_H_
