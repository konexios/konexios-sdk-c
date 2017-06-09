/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "ssl/ssl.h"
#include <wolfssl/ssl.h>
#include <arrow/mem.h>
#include <debug.h>
#include <unint.h>
#include <bsd/socket.h>
#ifdef DEBUG_WOLFSSL
#include <wolfcrypt/logging.h>
#endif

WOLFSSL_METHOD  *method;
WOLFSSL_CTX     *ctx;
WOLFSSL         *ssl;
static int _socket = -1;

#ifdef DEBUG_WOLFSSL
static void cli_wolfSSL_Logging_cb(const int logLevel,
                                  const char *const logMessage) {
    DBG("[http]:%d (%s)", logLevel, logMessage);
}
#endif

static int recv_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    SSP_PARAMETER_NOT_USED(vp);
//    int *sock = (int *)vp;
    if ( sz < 0 ) return sz;
    uint32_t got = 0;
    got = (uint32_t)recv(_socket, buf, (uint32_t)sz, 0);
//    DBG("recv ssl %d [%d]", got, sz);
    if (got == 0)  return -2;  // IO_ERR_WANT_READ;
    return (int)got;
}

static int send_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    SSP_PARAMETER_NOT_USED(vp);
//    int *sock = (int *)vp;
    if ( sz < 0 ) return sz;
    uint32_t sent = 0;
    sent = (uint32_t)send(_socket, buf, (uint32_t)sz, 0);
//    DBG("send ssl %d [%d]", sent, sz);
    if (sent == 0)
        return -2;  // IO_ERR_WANT_WRITE
    return (int)sent;
}

int __attribute__((weak)) ssl_connect(int sock) {
	wolfSSL_Init();
	_socket = sock;
	DBG("init ssl connect %d", sock)
	method = wolfTLSv1_2_client_method();
	ctx = wolfSSL_CTX_new(method);
	if ( ctx == NULL) {
		DBG("unable to get ctx");
	}
	wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
	wolfSSL_SetIORecv(ctx, recv_ssl);
	wolfSSL_SetIOSend(ctx, send_ssl);

	ssl = wolfSSL_new(ctx);
	if (ssl == NULL) {
		DBG("oops, bad SSL ptr");
	}
	wolfSSL_SetIOReadCtx(ssl, (void*)&_socket);
	wolfSSL_SetIOWriteCtx(ssl, (void*)&_socket);
#ifdef DEBUG_WOLFSSL
    wolfSSL_SetLoggingCb(cli_wolfSSL_Logging_cb);
    wolfSSL_Debugging_ON();
#endif
	int err = wolfSSL_connect(ssl);
	if (err != SSL_SUCCESS) {
		DBG("SSL connect fail");
		return -1;
	} else {
		DBG("SSL connect done");
	}
  return 0;
}

int __attribute__((weak)) ssl_recv(int sock, char *data, int len) {
//	DBG("ssl r[%d]", len);
	return wolfSSL_read(ssl, data, (int)len);
}

int __attribute__((weak)) ssl_send(int sock, char* data, int length) {
//	DBG("ssl w[%d]", length);
	return wolfSSL_write(ssl, data, (int)length);
}

int __attribute__((weak)) ssl_close(int sock) {
	SSP_PARAMETER_NOT_USED(sock);
	DBG("close ssl");
	wolfSSL_free(ssl);
	wolfSSL_CTX_free(ctx);
	wolfSSL_Cleanup();
	DBG("close done");
	return 0;
}
