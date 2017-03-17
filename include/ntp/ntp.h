/*
 * ntp.h
 *
 *  Created on: 29 oct 2016
 *      Author: ddemidov
 */

#ifndef ARROW_NTP_H_
#define ARROW_NTP_H_

#ifndef NTP_DEFAULT_SERVER
#define NTP_DEFAULT_SERVER "0.pool.ntp.org"
#endif

#ifndef NTP_DEFAULT_PORT
#define NTP_DEFAULT_PORT 123
#endif

#ifndef NTP_DEFAULT_TIMEOUT
#define NTP_DEFAULT_TIMEOUT 4000
#endif
#include <unint.h>

int ntp_set_time_cycle();
int ntp_set_time_common(const char *server, uint16_t port, int timeout, int try_times);

#endif /* ARROW_NTP_H_ */
