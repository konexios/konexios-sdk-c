/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_INT_H_
#define ACN_SDK_C_INT_H_

#include <config.h>

#if defined(ARCH_TYPE)
#include "sys/arch/type.h"
#else
# include <inttypes.h>
# include <sys/types.h>
#endif


#if defined(__USE_STD__)
#include <stdbool.h>
#else
typedef int bool;
#define false 0
#define true 1
#endif

#endif // ACN_SDK_C_INT_H_
