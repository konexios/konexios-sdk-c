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
#include <ntp/client.h>
#if defined(__MBED__)
#define SLEEP(ms) wait_ms(ms)
#elif defined(__linux__) || defined(__semiconductor__)
#include <unistd.h>
#define SLEEP(ms) usleep(ms*1000)
#elif defined(__XCC__)
#include <qcom_time.h>
#define SLEEP(ms) qcom_thread_msleep(ms)
#elif defined(__senseability__)
#define SLEEP CyDelay
#endif

int ntp_set_time_cycle() {
    return ntp_set_time_common(NTP_DEFAULT_SERVER, NTP_DEFAULT_PORT, NTP_DEFAULT_TIMEOUT, -1);
}

int ntp_set_time_common(
        const char *server,
        uint16_t port,
        int timeout,
        int try_times) {
    int i=0;
    do {
        while( ntp_set_time(server, port, timeout) != NTP_OK ) {
            DBG("NTP set time fail...");
            msleep(1000);
            if ( try_times >= 0 && i++ >= try_times ) return -1;
        }
        DBG(" time diff %d %d ", (int)time(NULL), (int)build_time());
    } while(time(NULL) <= build_time());
    return 0;
}
