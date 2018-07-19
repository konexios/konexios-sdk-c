/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "ntp/ntp.h"
#include <debug.h>
#include <time/time.h>
#include <sys/watchdog.h>
#include <ntp/client.h>


int ntp_set_time_cycle(void) {
    return ntp_set_time_common(NTP_DEFAULT_SERVER, NTP_DEFAULT_PORT, NTP_DEFAULT_TIMEOUT, ARROW_MAX_RETRY);
}

int ntp_set_time_common(
        const char *server,
        uint16_t port,
        int timeout,
        int try_times) {
    wdt_feed();
    time_t _build_t = (int)build_time();
    int i=0;
    do {
        while( ntp_set_time(server, port, (uint32_t)timeout) != NTP_OK ) {
            DBG("NTP set time fail...");
            msleep(1000);
            if ( try_times >= 0 && i++ >= try_times ) return -1;
        }

        DBG(" time diff %d %d ", (int)time(NULL), _build_t);
    } while(time(NULL) <= _build_t - 86400);
    return 0;
}
