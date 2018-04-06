 /* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef ACN_SDK_C_HTTP_CLIENT_H_
#define ACN_SDK_C_HTTP_CLIENT_H_

#include <http/request.h>
#include <http/response.h>

#include <data/ringbuffer.h>

#define LINE_CHUNK 40

typedef struct __session_flags {
  int _new   : 16;
  int _close : 16;
} __session_flags_t;

typedef int (*rw_func)(void *, uint8_t *, uint16_t);

typedef struct {
  int sock;
  uint32_t timeout;
  int response_code;
  __session_flags_t flags;
  ring_buffer_t  *queue;
  rw_func         _r_func;
  rw_func         _w_func;
} http_client_t;

void http_session_close_set(http_client_t *cli, bool mode);
void http_session_close_now(http_client_t *cli);
bool http_session_close(http_client_t *cli);

void http_client_init(http_client_t *cli);
void http_client_free(http_client_t *cli);

int http_client_do(http_client_t *cli, http_request_t *req, http_response_t *res);

#endif /* ACN_SDK_C_HTTP_CLIENT_H_ */
