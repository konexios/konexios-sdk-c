/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef HTTPCLIENT_ROUTINE_H_
#define HTTPCLIENT_ROUTINE_H_


#include <http/request.h>
#include <http/response.h>
#include <http/client.h>

typedef void(*http_request_init_cb)(http_request_t *request, void *arg);
typedef int (*http_response_process_cb)(http_response_t *response, void *arg);

#if defined(__cplusplus)
extern "C" {
#endif

// Setup the default settings for the 'client' (This is the session)
int http_init(void);

typedef void(*response_init_f)(http_request_t *request, void *arg);
typedef int (*response_proc_f)(http_response_t *response, void *arg);


http_client_t *current_client(void);

// Set the flag to keep the lower socked open or close it
// when a close() is called on the 'sesison'
// TODO: Break this in to layers, this session+socket logic
//      is crappy and confusing.
void http_session_keep_active(bool active);

// Set the timeout of recv for the already open client
void http_set_recv_timeout_ms(int ms);

// This is the main routine to init, send, receive HTTP
int http_routine(http_request_init_cb req_init, void *arg_init,
        http_response_process_cb resp_proc, void *arg_proc);

// Get the last response code (similar to errno).  This should
// be fine since there is only one client.  Really this all
// needs to be re-written
int http_last_response_code();

// This ends an HTTP session and actually closes the
// lower level socket
int http_end(void);

// This 'frees' the ring buffer and queue.  After calling
// this, http_init() must be called again
int http_done(void);

#if defined(__cplusplus)
}
#endif


//
// A dirty macro to call http_routine and print an error on failure
//
// init=request init callback, i_arg=arguments to init callback
// proc=process response callback, p_arg=arguments to process callback
// ...= args to DBG on failure
//
#define STD_ROUTINE(init, i_arg, proc, p_arg, ...) { \
  int ret = http_routine(init, i_arg, proc, p_arg); \
  if ( ret < 0 ) { \
      DBG("Error:" __VA_ARGS__); \
  } \
  return ret; \
}

#endif /* HTTPCLIENT_ROUTINE_H_ */
