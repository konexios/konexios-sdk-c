/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_SYS_MUTEX_H_
#define ACN_SDK_C_SYS_MUTEX_H_

#include <konexios_config.h>

#if defined(KONEXIOS_THREAD)
typedef void konexios_mutex;

int konexios_mutex_init(konexios_mutex **mutex);
int konexios_mutex_deinit(konexios_mutex *mutex);
int konexios_mutex_lock(konexios_mutex *mutex);
int konexios_mutex_unlock(konexios_mutex *mutex);

#define CRITICAL_SECTION_START(mutex)   konexios_mutex_lock(mutex)
#define CRITICAL_SECTION_STOP(mutex)    konexios_mutex_unlock(mutex)
#else
#define CRITICAL_SECTION_START(...)
#define CRITICAL_SECTION_END(...)
#endif

#endif
