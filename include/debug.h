/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <config.h>

//Debug is disabled by default
#ifdef DEBUG
#ifdef _ARIS_
#include "TRACE_USE.h"
#elif defined(__XCC__)
#include <qcom/base.h>
#define DBG(x, ...) A_PRINTF("[DBG]"x"\r\n", ##__VA_ARGS__);
#define WARN(x, ...) A_PRINTF("[WARN]"x"\r\n", ##__VA_ARGS__);
#define ERR(x, ...)  A_PRINTF("[ERR]"x"\r\n", ##__VA_ARGS__);
    
#elif defined(__senseability__)
#include "platforms/senseability/trace.h"
#else
#include <stdio.h>
#define DBG(x, ...) printf("[DBG]"x"\r\n", ##__VA_ARGS__);
#define WARN(x, ...) printf("[WARN]"x"\r\n", ##__VA_ARGS__);
#define ERR(x, ...)  printf("[ERR]"x"\r\n", ##__VA_ARGS__);
#endif
#else
//Disable debug
#define DBG(x, ...)
#define WARN(x, ...)
#define ERR(x, ...)

#endif

#if defined(__cplusplus)
}
#endif

#endif
