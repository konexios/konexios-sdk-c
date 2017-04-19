/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "time/time.h"

void get_time(char *ts) {
    // FIXME implementation
}

static void get_time_of_day(struct timeval *tval) {
    // FIXME implementation
}

int gettimeofday(struct timeval *__restrict __p,  void *__restrict __tz) {
    // FIXME implementation
    return 0;
}

time_t time(time_t *timer) {
    // FIXME implementation
    return 0;
}

int stime(time_t *timer) {
    // FIXME implementation
    return 0;
}

int msleep(int m_sec) {
  // FIXME implementation
  return 0;
}
