/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#define MODULE_NAME "HTTP_Client"

#include <http/client.h>
#include <config.h>
#include <debug.h>
#include <bsd/socket.h>
#include <time/time.h>
#include <arrow/mem.h>
#if defined(_ARIS_)
# if defined(ETH_MODE)
#  include "nx_api.h"
# endif
#elif defined(__MBED__)
#elif defined(__XCC__)
#define WOLFSSL SSL
#define WOLFSSL_CTX SSL_CTX
#define wolfSSL_write qcom_SSL_write
#define wolfSSL_read qcom_SSL_read
#define IPPROTO_TCP 0
#define SSL_SUCCESS 0
#endif

#if !defined(MAX_BUFFER_SIZE)
#define MAX_BUFFER_SIZE 1024
#endif

#define CHUNK_SIZE 256

#define CHECK_CONN_ERR(ret) \
    if ( ret < 0 ) { \
      DBG("Connection error (%d)", ret); \
      return ret; \
    }

#define PRTCL_ERR() \
  { \
    DBG("Protocol error"); \
    soc_close(cli->sock); \
    return -1; \
  }

#define client_send(buf, size, cli) (*(cli->_w_func))((uint8_t*)(buf), (uint16_t)(size), (cli))
#define client_recv(buf, size, cli) (*(cli->_r_func))((uint8_t*)(buf), (uint16_t)(size), (cli))


#if !defined(__XCC__)
static int recv_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    http_client_t *cli = (http_client_t *)vp;
    //      tcp->set_blocking(true, 15000);
    if ( sz < 0 ) return sz;
    uint32_t got = 0;
    got = (uint32_t)recv(cli->sock, buf, (uint32_t)sz, 0);
    HTTP_DBG("recv ssl %d [%d]", got, sz);
    if (got == 0)  return -2;  // IO_ERR_WANT_READ;
    return (int)got;
}


static int send_ssl(WOLFSSL *wsl, char* buf, int sz, void* vp) {
    SSP_PARAMETER_NOT_USED(wsl);
    http_client_t *cli = (http_client_t *)vp;
    if ( sz < 0 ) return sz;
    uint32_t sent = 0;
    sent = (uint32_t)send(cli->sock, buf, (uint32_t)sz, 0);
    HTTP_DBG("send ssl %d [%d]", sent, sz);
    if (sent == 0)
        return -2;  // IO_ERR_WANT_WRITE
    return (int)sent;
}
#endif

static int simple_read(uint8_t *buf, uint16_t len, void *c) {
    http_client_t *cli = (http_client_t *)c;
    int ret;
    ret = recv(cli->sock, (char*)buf, (int)len, 0);
    if (ret > 0) buf[ret] = 0x00;
    HTTP_DBG("%d|%s|", ret, buf);
    return ret;
}

static int simple_write(uint8_t *buf, uint16_t len, void *c) {
    http_client_t *cli = (http_client_t *)c;
    if ( !len ) len = (uint16_t)strlen((char*)buf);
    HTTP_DBG("%d|%s|", len, buf);
    return send(cli->sock, (char*)buf, (int)len, 0);
}

static int ssl_read(uint8_t *buf, uint16_t len, void *c) {
    http_client_t *cli = (http_client_t *)c;
    int ret = wolfSSL_read(cli->ssl, buf, (int)len);
    if (ret > 0) buf[ret] = 0x00;
    HTTP_DBG("[%d]{%s}", ret, buf);
    return ret;
}

static int ssl_write(uint8_t *buf, uint16_t len, void *c) {
    http_client_t *cli = (http_client_t *)c;
    if ( !len && buf ) len = (uint16_t)strlen((char*)buf);
    HTTP_DBG("[%d]|%s|",len, buf);
    int ret = wolfSSL_write(cli->ssl, buf, (int)len);
    return ret;
}

#ifdef DEBUG_WOLFSSL
static void cli_wolfSSL_Logging_cb(const int logLevel,
                                  const char *const logMessage) {
    DBG("[http]:%d (%s)", logLevel, logMessage);
}
#endif

void http_client_init(http_client_t *cli) {
    cli->sock = -1;
    cli->response_code = 0;
    cli->timeout = 5000;
    cli->_r_func = simple_read;
    cli->_w_func = simple_write;
#if !defined(__XCC__)
    wolfSSL_Init();
#endif
#ifdef DEBUG_WOLFSSL
//    wolfSSL_SetLoggingCb(cli_wolfSSL_Logging_cb);
//    wolfSSL_Debugging_ON();
#endif
    cli->ctx = NULL;
    cli->ssl = NULL;
}

void http_client_free(http_client_t *cli) {
#if defined(__XCC__)
  if (cli->ssl) {
    qcom_SSL_shutdown(cli->ssl);
    qcom_SSL_ctx_free(cli->ctx);
  }
#else
    wolfSSL_free(cli->ssl);
    wolfSSL_CTX_free(cli->ctx);
    wolfSSL_Cleanup();
#endif
    cli->ssl = NULL;
    cli->ctx = NULL;
}

static int send_start(http_client_t *cli, http_request_t *req) {
    char buf[CHUNK_SIZE];
    int ret;
    ret = snprintf(buf, CHUNK_SIZE, "%s %s", req->meth, req->uri);
    buf[ret] = '\0';
    if ( req->query ) {
        char queryString[CHUNK_SIZE];
        strcpy(queryString, "?");
        http_query_t *query = req->query;
        while ( query ) {
            strcat(queryString, (char*)query->key);
            strcat(queryString, "=");
            strcat(queryString, (char*)query->value);
            strcat(buf, queryString);
            strcpy(queryString, "&");
            query = query->next;
        }
    }
    strcat(buf, " HTTP/1.1\r\n");
    if ( (ret = client_send(buf, 0, cli)) < 0 ) {
        return ret;
    }
    ret = snprintf(buf, CHUNK_SIZE, "Host: %s:%d\r\n", req->host, req->port);
    buf[ret] = '\0';
    if ( (ret = client_send(buf, 0, cli)) < 0 ) {
        return ret;
    }
    return 0;
}

static int send_header(http_client_t *cli, http_request_t *req) {
    char buf[CHUNK_SIZE];
    int ret;
    if ( req->payload.buf && req->payload.size > 0 ) {
        if ( req->is_chunked ) {
            ret = client_send("Transfer-Encoding: chunked\r\n", 0, cli);
        } else {
            ret = snprintf(buf, sizeof(buf), "Content-Length: %lu\r\n", (long unsigned int)req->payload.size);
            buf[ret] = '\0';
            ret = client_send(buf, 0, cli);
        }
        if ( ret < 0 ) return ret;
        ret = snprintf(buf, sizeof(buf), "Content-Type: %s\r\n", req->content_type.value);
        buf[ret] = '\0';
        if ( (ret = client_send(buf, 0, cli)) < 0 ) return ret;
    }

    http_header_t *head = req->header;
    while( head ) {
        strcpy(buf, (char*)head->key);
        strcat(buf, ": ");
        strcat(buf, (char*)head->value);
        strcat(buf, "\r\n");
        if ( (ret = client_send(buf, 0, cli)) < 0 ) return ret;
        head = head->next;
    }
    return client_send((uint8_t*)"\r\n", 0, cli);
}

static int send_payload(http_client_t *cli, http_request_t *req) {
    if ( req->payload.buf && req->payload.size > 0 ) {
        if ( req->is_chunked ) {
            char *data = (char*)req->payload.buf;
            int len = (int)req->payload.size;
            int trData = 0;
            while ( len >= 0 ) {
                char buf[6];
                int chunk = len > CHUNK_SIZE ? CHUNK_SIZE : len;
                int ret = sprintf(buf, "%02X\r\n", chunk);
                client_send(buf, ret, cli);
                if ( chunk ) client_send(data + trData, chunk, cli);
                trData += chunk;
                len -= chunk;
                client_send("\r\n", 0, cli);
                if ( !chunk ) break;
            }
            return 0;
        } else {
          int ret = client_send(req->payload.buf, req->payload.size, cli);
          return ret;
        }
    }
    return -1;
}

static int receive_response(http_client_t *cli, http_response_t *res, char *buf, uint32_t *len) {
    int ret;
    if ( (ret = client_recv(buf, 20, cli)) < 0 ) return ret;
    *len = (uint32_t)ret;
    char* crlfPtr = strstr(buf, "\r\n");

    while( crlfPtr == NULL ) {
        if( *len < CHUNK_SIZE - 1 ) {
            uint32_t newTrfLen;
            if ( (ret = client_recv(buf + *len, 10, cli)) < 0 ) return ret;
            newTrfLen = (uint32_t)ret;
            *len += newTrfLen;
        } else {
            return -1;
        }
        crlfPtr = strstr(buf, "\r\n");
    }

    int crlfPos = crlfPtr - buf;
    if ( crlfPos < 0 && crlfPos > (int)(*len) )
        return -1;
    buf[crlfPos] = '\0';
    DBG("resp: {%s}", buf);

    if( sscanf(buf, "HTTP/1.1 %4d", &res->m_httpResponseCode) != 1 ) {
        DBG("Not a correct HTTP answer : %s\n", buf);
        return -1;
    }

    if ( *len < (uint32_t)crlfPos + 2 ) {
        DBG("receive_response memmove warning [%08x] %d, %d", (int)buf, crlfPos, *len);
    }
    memmove(buf, buf+crlfPos+2, *len - (uint32_t)(crlfPos + 2) + 1 ); //Be sure to move NULL-terminating char as well
    *len -= (uint32_t)(crlfPos + 2);

    if( (res->m_httpResponseCode < 200) || (res->m_httpResponseCode >= 300) ) {
        //Did not return a 2xx code; TODO fetch headers/(&data?) anyway and implement a mean of writing/reading headers
        DBG("Response code %d", res->m_httpResponseCode);
        HTTP_DBG("Protocol error");
        return -1;
    }
    cli->response_code = res->m_httpResponseCode;
    return 0;
}

int http_client_do(http_client_t *cli, http_request_t *req, http_response_t *res) {
    int ret;
    memset(res, 0x00, sizeof(http_response_t));
    ret = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( ret < 0 ) return ret;
    cli->sock = ret;
    struct sockaddr_in serv;
    struct hostent *serv_resolve;
    serv_resolve = gethostbyname((char*)req->host);
    if (serv_resolve == NULL) {
        DBG("ERROR, no such host");
        return -1;
    }
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = PF_INET;
    bcopy((char *)serv_resolve->h_addr,
            (char *)&serv.sin_addr.s_addr,
            (uint32_t)serv_resolve->h_length);
    serv.sin_port = htons(req->port);
//    serv.sin_addr.s_addr = serv.sin_addr.s_addr;

    struct timeval tv;
    tv.tv_sec =     (time_t)        ( cli->timeout / 1000 );
    tv.tv_usec =    (suseconds_t)   (( cli->timeout % 1000 ) * 1000);
    setsockopt(cli->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    ret = connect(cli->sock, (struct sockaddr*)&serv, sizeof(serv));
    if ( ret < 0 ) {
        DBG("connect fail");
        soc_close(cli->sock);
        return -1;
    }
    HTTP_DBG("connect done");

    if ( req->is_cipher ) {
      cli->method = wolfTLSv1_2_client_method();
#if defined(__XCC__)
      cli->ctx = qcom_SSL_ctx_new(SSL_CLIENT, SSL_INBUF_SIZE, SSL_OUTBUF_SIZE, 0);
      if ( cli->ctx == NULL) {
          DBG("unable to get ctx");
          soc_close(cli->sock);
          return -1;
      }
      cli->ssl = qcom_SSL_new(cli->ctx);
      qcom_SSL_set_fd(cli->ssl, cli->sock);
      if (cli->ssl == NULL) {
          DBG("oops, bad SSL ptr");
          soc_close(cli->sock);
          return -1;
      }
      int err = qcom_SSL_connect(cli->ssl);
      if (err < 0) {
          DBG("SSL connect fail %d", ret);
          soc_close(cli->sock);
          return err;
      }
#else
      cli->ctx = wolfSSL_CTX_new(cli->method);
      if ( cli->ctx == NULL) {
          DBG("unable to get ctx");
      }
      wolfSSL_CTX_set_verify(cli->ctx, SSL_VERIFY_NONE, 0);
      wolfSSL_SetIORecv(cli->ctx, recv_ssl);
      wolfSSL_SetIOSend(cli->ctx, send_ssl);

      cli->ssl = wolfSSL_new(cli->ctx);
      if (cli->ssl == NULL) {
          DBG("oops, bad SSL ptr");
      }
      wolfSSL_SetIOReadCtx(cli->ssl, (void*)cli);
      wolfSSL_SetIOWriteCtx(cli->ssl, (void*)cli);
      //        wolfSSL_Debugging_ON();
      int err = wolfSSL_connect(cli->ssl);
      if (err != SSL_SUCCESS) {
          DBG("SSL connect fail");
      } else {
          HTTP_DBG("SSL connect done");
      }
#endif
        cli->_r_func = ssl_read;
        cli->_w_func = ssl_write;
    } else {
        cli->_r_func = simple_read;
        cli->_w_func = simple_write;
    }

    if ( send_start(cli, req) < 0 ) {
        soc_close(cli->sock);
        DBG("send start fail");
        return -1;
    }

    if ( send_header(cli, req) < 0 ) {
        soc_close(cli->sock);
        DBG("send header fail");
        return -1;
    }

    if ( req->payload.buf ) {
        if ( send_payload(cli, req) < 0 ) {
            DBG("send payload fail");
            soc_close(cli->sock);
            return -1;
        }
    }

    char buf[CHUNK_SIZE];

    HTTP_DBG("Receiving response");

    uint32_t trfLen;
    ret = receive_response(cli, res, buf, &trfLen);
    if ( ret < 0 ) {
        DBG("Connection error (%d)", ret);
        soc_close(cli->sock);
        return -1;
    }

    HTTP_DBG("Reading headers %d", trfLen);
    char *crlfPtr;
    int crlfPos;
    res->header = NULL;
    res->content_type.value = NULL;
    req->content_type.key = NULL;
    req->content_type.next = NULL;
    res->payload.size = 0;
    res->payload.buf = 0;
    res->is_chunked = 0;

    int recvContentLength = -1;
    //Now get headers
    while( 1 ) {
        crlfPtr = strstr(buf, "\r\n");
        if(crlfPtr == NULL) {
            if( trfLen < CHUNK_SIZE - 1 ) {
                uint32_t newTrfLen;
                ret = client_recv(buf + trfLen, 40, cli);
                CHECK_CONN_ERR(ret);
                newTrfLen = (uint32_t)ret;
                trfLen += newTrfLen;
                HTTP_DBG("Read %d chars; In buf: [%s]", newTrfLen, buf);
                continue;
            } else {
                PRTCL_ERR();
            }
        }

        crlfPos = crlfPtr - buf;

        if(crlfPos == 0) {
            HTTP_DBG("Headers read done");
            memmove(buf, &buf[2], trfLen - 2 + 1); //Be sure to move NULL-terminating char as well
            trfLen -= 2;
            break;
        } else if ( crlfPos < 0 ) CHECK_CONN_ERR(-1);

        buf[crlfPos] = '\0';
        char key[CHUNK_SIZE];
        char value[CHUNK_SIZE];

        int n = sscanf(buf, "%256[^:]: %256[^\r\n]", key, value);
        if ( n == 2 ) {
            HTTP_DBG("Read header : %s: %s", key, value);
            if( !strcmp(key, "Content-Length") ) {
                sscanf(value, "%8d", &recvContentLength);
            } else if( !strcmp(key, "Transfer-Encoding") ) {
                if( !strcmp(value, "Chunked") || !strcmp(value, "chunked") )
                    res->is_chunked = 1;
            } else if( !strcmp(key, "Content-Type") ) {
                http_response_set_content_type(res, value);
            } else {
                http_response_add_header(res, key, value);
            }
            memmove(buf, crlfPtr+2, trfLen - (uint32_t)(crlfPos + 2) + 1);
            trfLen -= (uint32_t)(crlfPos + 2);
        } else {
            DBG("Could not parse header");
            PRTCL_ERR();
        }
    }

    // return resp; //

    uint32_t chunk_len;
    HTTP_DBG("get payload form buf: %d", trfLen);
    HTTP_DBG("get payload form buf: [%s]", buf);
    do {
        if ( res->is_chunked ) {
            while( (crlfPtr = strstr(buf, "\r\n")) == NULL ) {
                if ( trfLen + 10 > CHUNK_SIZE ) {
                    memmove(buf, buf+10, trfLen-10);
                    trfLen -= 10;
                }
                uint32_t newTrfLen = 0;
                ret = client_recv(buf+trfLen, 10, cli);
                if ( ret > 0 ) newTrfLen = (uint32_t)ret;
                trfLen += newTrfLen;
            }
            ret = sscanf(buf, "%4x\r\n", (unsigned int*)&chunk_len);
            if ( ret != 1 ) {
                memmove(buf, crlfPtr+2, trfLen - (uint32_t)crlfPos);
                trfLen -= (uint32_t)crlfPos;
                chunk_len = 0;
                return -1; // fail
            }
            HTTP_DBG("detect chunk %d, %d", chunk_len, ret);
            crlfPtr = strstr(buf, "\r\n");
            crlfPos = crlfPtr + 2 - buf;
            memmove(buf, crlfPtr+2, trfLen - (uint32_t)crlfPos);
            trfLen -= (uint32_t)crlfPos;
        } else {
            chunk_len = MAX_BUFFER_SIZE - trfLen;
        }
        if ( !chunk_len ) break;
        while ( chunk_len ) {
            uint32_t need_to_read = CHUNK_SIZE-10;
            if ( (int)chunk_len < CHUNK_SIZE-10) need_to_read = chunk_len;
            HTTP_DBG("need to read %d/%d", need_to_read, trfLen);
            while ( (int)trfLen < (int)need_to_read ) {
                uint32_t newTrfLen = 0;
                HTTP_DBG("get chunk add %d", need_to_read-trfLen);
                ret = client_recv(buf+trfLen, need_to_read-trfLen, cli);
                if ( ret >= 0 ) newTrfLen = (uint32_t)ret;
                else { // ret < 0 - error
                    need_to_read = trfLen;
                    chunk_len = need_to_read;
                    newTrfLen = 0;
                }
                trfLen += newTrfLen;
            }
            HTTP_DBG("add payload{%d:%s}", need_to_read, buf);
            http_response_add_payload(res, buf, need_to_read);
            if ( trfLen == need_to_read ) {
                trfLen = 0;
                buf[0] = 0;
            } else {
                memmove(buf, buf + need_to_read, trfLen - need_to_read);
                trfLen -= need_to_read;
            }
            chunk_len -= need_to_read;
        }
    } while(1);

    HTTP_DBG("body{%s}", res->payload.buf);
    soc_close(cli->sock);
    return 0;
}
