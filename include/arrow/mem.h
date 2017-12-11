/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_MEM_H_
#define ACN_SDK_C_MEM_H_

#include <config.h>
#include <unint.h>
#if defined(__USE_STD__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if GCC_VERSION <= 50201
# include <sys/cdefs.h>
#endif
#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 17 && __GLIBC_MINOR__ < 23
# include <features.h>
#else
# include <sys/features.h>
#endif
# include <stddef.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <strings.h>
#endif
#if defined(ARCH_MEM)
# include "sys/arch_mem.h"
#endif

#if defined(__xtensa__) || defined(__XCC__)
# define __attribute_packed__
#else
# define __attribute_packed__ __attribute__((__packed__))
#endif

#endif  // ACN_SDK_C_MEM_H_
