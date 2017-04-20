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

int msleep(int m_sec);
void get_time(char *ts);
time_t build_time();
#if !defined(__MBED__)
void set_time(time_t t);
#endif

# endif // COMMON_TIME_TIME_H_
