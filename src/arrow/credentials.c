/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "arrow/credentials.h"
#include <sys/mem.h>

wifi_credentials_t wifi_crds[] = {
#if defined(DEFAULT_WIFI_SSID) && defined(DEFAULT_WIFI_PASS) && defined(DEFAULT_WIFI_SEC)
    { DEFAULT_WIFI_SSID,
      DEFAULT_WIFI_PASS,
      DEFAULT_WIFI_SEC },
#endif
#if defined(DEFAULT_WIFI_SSID_1) && defined(DEFAULT_WIFI_PASS_1) && defined(DEFAULT_WIFI_SEC_1)
    { DEFAULT_WIFI_SSID_1,
      DEFAULT_WIFI_PASS_1,
      DEFAULT_WIFI_SEC_1 },
#endif
#if defined(DEFAULT_WIFI_SSID_2) && defined(DEFAULT_WIFI_PASS_2) && defined(DEFAULT_WIFI_SEC_2)
    { DEFAULT_WIFI_SSID_2,
      DEFAULT_WIFI_PASS_2,
      DEFAULT_WIFI_SEC_2 },
#endif
#if defined(DEFAULT_WIFI_SSID_3) && defined(DEFAULT_WIFI_PASS_3) && defined(DEFAULT_WIFI_SEC_3)
     { DEFAULT_WIFI_SSID_3,
       DEFAULT_WIFI_PASS_3,
       DEFAULT_WIFI_SEC_3 },
#endif
    {NULL, NULL, 0}
};

uint32_t credentials_qnt() {
    return sizeof(wifi_crds) / sizeof(wifi_credentials_t) - 1;
}

wifi_credentials_t *credentials_get(int index) {
    return wifi_crds + index;
}

wifi_credentials_t *credentials_next() {
    static int count = 0;
    uint32_t qnt = credentials_qnt();
    if ( qnt ) {
        wifi_credentials_t *p = wifi_crds + count;
        count = ( count + 1 ) % credentials_qnt();
        return p;
    }
    return NULL;
}

#include <arrow/storage.h>
static arrow_host_t __api_host;
static arrow_host_t __mqtt_host;

arrow_host_t *arrow_api_host(void) {
    return &__api_host;
}

arrow_host_t *arrow_mqtt_host(void) {
    return &__mqtt_host;
}

void arrow_hosts_init() {
    restore_api_address(&__api_host);
    restore_mqtt_address(&__mqtt_host);
}
