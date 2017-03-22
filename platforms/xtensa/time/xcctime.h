/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ARROW_KRONOS_C_SDK_XCCTIME_H_
#define _ARROW_KRONOS_C_SDK_XCCTIME_H_

time_t mktime(struct tm *timeptr);
char *strptime (const char *buf, const char *format, struct tm *tm);
int stime(time_t *t);

#endif
