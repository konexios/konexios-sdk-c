/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_MEM_H_
#define ACN_SDK_C_MEM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <config.h>
#include <sys/type.h>
#if defined(__USE_STD__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if GCC_VERSION <= 50201
# include <sys/cdefs.h>
#endif
# include <features.h>
# include <stddef.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <strings.h>
#endif
#if defined(ARCH_MEM)
# include "sys/arch/mem.h"
#endif

#if defined(__xtensa__) || defined(__XCC__)
# define __attribute_packed__
#else
# define __attribute_packed__ __attribute__((__packed__))
#endif

#if defined(__cplusplus)
}
#endif

#endif  // ACN_SDK_C_MEM_H_
