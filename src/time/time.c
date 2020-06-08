/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "time/time.h"
#include <sys/mem.h>

time_t build_time(void) {
  static const char *built = __DATE__ " " __TIME__;
  struct tm t = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  const char *ret = (const char *)strptime(built, "%b %d %Y %H:%M:%S", &t);
  if ( ret ) {
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    return mktime(&t);
  }
  return 0;
}

int timestamp_less(acn_timestamp_t *t1, acn_timestamp_t *t2) {
    uint32_t *data1 = (uint32_t*)t1;
    uint32_t *data2 = (uint32_t*)t2;
    if ( *data1 < *data2 ) return 1;
    if ( *data1 == *data2 ) {
        uint32_t *time1 = data1 + 1;
        uint32_t *time2 = data2 + 1;
        if ( *time1 < *time2 ) return 1;
    }
    return 0;
}

int timestamp_is_empty(acn_timestamp_t *t) {
    uint32_t *data = (uint32_t*)t;
    if ( *data ) return 0;
    data++;
    if ( *data ) return 0;
    return 1;
}

void timestamp_string(acn_timestamp_t *ts, char *s) {
    snprintf(s, 29,
             "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             ts->year,
             ts->mon,
             ts->day,
             ts->hour,
             ts->min,
             ts->sec,
             ts->msec);
}

void __attribute_weak__ timestamp(acn_timestamp_t *ts) {
    struct tm *tmp;
    int ms;
    time_t s = time(NULL);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tmp = gmtime(&s);
    ms = (tv.tv_usec/1000)%1000;
    ts->year = 1900 + tmp->tm_year;
    ts->mon = 1 + tmp->tm_mon;
    ts->day = tmp->tm_mday;
    ts->hour = tmp->tm_hour;
    ts->min = tmp->tm_min;
    ts->sec = tmp->tm_sec;
    ts->msec = ms;
}

void __attribute_weak__ get_time(char *s) {
    acn_timestamp_t ts;
    timestamp(&ts);
    timestamp_string(&ts, s);
}
