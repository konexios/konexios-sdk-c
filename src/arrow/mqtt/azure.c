/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include <arrow/mqtt.h>
#include <arrow/sign.h>
#include <debug.h>

#define USE_STATIC
#include <data/chunk.h>

#if defined(__AZURE__)

#else
typedef void __dummy;
#endif
