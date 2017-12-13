/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef COMMON_TIME_TIME_H_
#define COMMON_TIME_TIME_H_

#include <config.h>
#include <sys/type.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(ARCH_TIME)
# include <sys/arch/time.h>
#else
# include <sys/time.h>
# include <time.h>
# if !defined(timerclear)
#  include <time/timer_functions.h>
# endif
#endif

int msleep(int m_sec);
time_t build_time(void);
void get_time(char *ts);
int stime(const time_t *t);

#if defined(__cplusplus)
}
#endif

# endif // COMMON_TIME_TIME_H_
