/* Copyright (c) 2018 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _ARROW_C_SDK_STATIC_ALLOC_H_
#define _ARROW_C_SDK_STATIC_ALLOC_H_

#include <sys/mem.h>
#include <data/static_buf.h>

#define static_object_pool_type(type, count) \
CREATE_BUFFER(staticbuf_##type, sizeof(type) * (count), sizeof(type))

#define static_alloc_size(type) static_buf_free_size(staticbuf_##type)
#define static_allocator(type)  (type *)static_buf_alloc(staticbuf_##type, sizeof(type));
#define static_free(type, ptr)  static_buf_free(staticbuf_##type, (ptr));

#endif  // _ARROW_C_SDK_STATIC_ALLOC_H_
