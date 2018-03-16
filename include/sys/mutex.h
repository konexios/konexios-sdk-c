/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_SYS_MUTEX_H_
#define ACN_SDK_C_SYS_MUTEX_H_

typedef void arrow_mutex;

int arrow_mutex_init(arrow_mutex **mutex);
int arrow_mutex_deinit(arrow_mutex *mutex);
int arrow_mutex_lock(arrow_mutex *mutex);
int arrow_mutex_unlock(arrow_mutex *mutex);

#endif
