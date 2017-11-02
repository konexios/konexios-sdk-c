/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#define MODULE_NAME "HTTP_Client"

#include "http/client.h"
#include <config.h>
#include <debug.h>
#include <bsd/socket.h>
#include <time/time.h>
#include <arrow/mem.h>
#if defined(_ARIS_)
# if defined(ETH_MODE)
#  include "nx_api.h"
# endif
#endif

#include <ssl/ssl.h>

void http_session_close_set(http_client_t *cli, bool mode) {
  cli->flags._close = mode;
}

bool http_session_close(http_client_t *cli) {
  return cli->flags._close;
}

#if !defined(MAX_BUFFER_SIZE)
#define MAX_BUFFER_SIZE 1024
#endif

#define CHUNK_SIZE 256

#define CHECK_CONN_ERR(ret) \
    if ( ret < 0 ) { \
      DBG("Connection error [%d] (%d)", __LINE__, ret); \
      return ret; \
    }

#define PRTCL_ERR() \
  { \
    DBG("Protocol error"); \
    return -1; \
  }

#define client_send(cli)                    (*(cli->_w_func))((cli), NULL, 0)
#define client_send_direct(cli, buf, size)  (*(cli->_w_func))((cli), (uint8_t*)(buf), (size))
#define client_recv(cli, size)              (*(cli->_r_func))((cli), NULL, (size))

#include <arrow/queue.h>

static queue_buffer_t http_queue = { { 0 }, 0, 0 };

#define QUEUE_POS(x) (ptrdiff_t)( (intptr_t)(x) - http_queue.shift - (intptr_t)http_queue.buffer )

static int simple_read(void *c, uint8_t *buf, uint16_t len) {
    http_client_t *cli = (http_client_t *)c;
    if ( ! buf ) buf = queue_wr_addr(cli->queue);
    int ret = recv(cli->sock, buf, (int)len, 0);
    queue_size_add(cli->queue, ret);
    HTTP_DBG("%d|%s|", ret, queue_rd_addr(cli->queue));
    return ret;
}

static char *__prep_write_function(http_client_t *cli, uint8_t *buf, uint16_t *len, int *direct) {
    char *start = NULL;
    if ( !buf ) {
        if ( direct ) *direct = 0;
        start = (char*)queue_rd_addr(cli->queue);
        if ( len ) *len = queue_size(cli->queue);
    } else {
        if ( direct ) *direct = 1;
        start = (char*)buf;
        if ( len && !( *len ) ) *len = strlen((char*)buf);
    }
    return start;
}

static int simple_write(void *c, uint8_t *buf, uint16_t len) {
    http_client_t *cli = (http_client_t *)c;
    int direct;
    char *start = __prep_write_function(cli, buf, &len, &direct);
    HTTP_DBG("%d|%s|", len, start);
    int ret = send(cli->sock, start, len, 0);
    if ( !direct ) queue_shift(cli->queue, len);
    return ret;
}

static int ssl_read(void *c, uint8_t *buf, uint16_t len) {
    http_client_t *cli = (http_client_t *)c;
    if ( !buf ) buf = queue_wr_addr(cli->queue);
    int ret = ssl_recv(cli->sock, (char*)buf, len);
    if ( ret > 0 ) queue_size_add(cli->queue, ret);
    HTTP_DBG("[%d]{%s}", ret, queue_rd_addr(cli->queue));
    return ret;
}

static int ssl_write(void *c, uint8_t *buf, uint16_t len) {
    http_client_t *cli = (http_client_t *)c;
    int direct;
    char *start = __prep_write_function(cli, buf, &len, &direct);
    HTTP_DBG("[%d]|%s|", len, start);
    int ret = ssl_send(cli->sock, start, len);
    if ( !direct ) queue_shift(cli->queue, len);
    return ret;
}

void http_client_init(http_client_t *cli) {
    cli->response_code = 0;
    cli->queue = &http_queue;
    if ( cli->flags._new ) {
        cli->sock = -1;
        cli->timeout = DEFAULT_API_TIMEOUT;
        cli->_r_func = simple_read;
        cli->_w_func = simple_write;
    }
}

void http_client_free(http_client_t *cli) {
  if ( cli->flags._close ) {
    if ( cli->sock >= 0 ) {
#if defined(HTTP_CIPHER)
      ssl_close(cli->sock);
#endif
      soc_close(cli->sock);
    }
    cli->flags._new = 1;
  } else {
    cli->flags._new = 0;
  }
}

#define HTTP_VERS " HTTP/1.1\r\n"
static int send_start(http_client_t *cli, http_request_t *req, queue_buffer_t *buf) {
    queue_clear(buf);
    int ret = queue_printf(buf, "%s %s", P_VALUE(req->meth), P_VALUE(req->uri));
    if ( req->query ) {
        char queryString[CHUNK_SIZE];
        strcpy(queryString, "?");
        http_query_t *query = req->query;
        while ( query ) {
          if ( (int)strlen(P_VALUE(query->key)) + (int)strlen(P_VALUE(query->value)) + 3 < (int)CHUNK_SIZE ) break;
            strcat(queryString, P_VALUE(query->key));
            strcat(queryString, "=");
            strcat(queryString, P_VALUE(query->value));
            if ( queue_capacity(buf) - sizeof(HTTP_VERS)-1 < strlen(queryString) ) break;
            queue_strcat(buf, queryString);
            strcpy(queryString, "&");
            query = query->next;
        }
    }
    queue_strcat(buf, HTTP_VERS);
    if ( (ret = client_send(cli)) < 0 ) {
        return ret;
    }
    ret = queue_printf(buf, "Host: %s:%d\r\n", P_VALUE(req->host), req->port);
    if ( (ret = client_send(cli)) < 0 ) {
        return ret;
    }
    return 0;
}

static int send_header(http_client_t *cli, http_request_t *req, queue_buffer_t *buf) {
    int ret;
    queue_clear(buf);
    if ( !IS_EMPTY(req->payload.buf) && req->payload.size > 0 ) {
        if ( req->is_chunked ) {
            ret = client_send_direct(cli, "Transfer-Encoding: chunked\r\n", 0);
        } else {
            ret = queue_printf(buf, "Content-Length: %lu\r\n", (long unsigned int)req->payload.size);
            if ( ret < 0 ) return ret;
            ret = client_send(cli);
        }
        queue_clear(buf);
        if ( ret < 0 ) return ret;
        ret = queue_printf(buf, "Content-Type: %s\r\n", P_VALUE(req->content_type.value));
        if ( ret < 0 ) return ret;
        if ( (ret = client_send(cli)) < 0 ) return ret;
    }
    http_header_t *head = req->header;
    while ( head ) {
        queue_clear(buf);
        ret = queue_printf(buf, "%s: %s\r\n", P_VALUE(head->key), P_VALUE(head->value));
    	if ( ret < 0 ) return ret;
        if ( (ret = client_send(cli)) < 0 ) return ret;
        head = head->next;
    }
    return client_send_direct(cli, "\r\n", 2);
}

static int send_payload(http_client_t *cli, http_request_t *req) {
    if ( !IS_EMPTY(req->payload.buf) && req->payload.size > 0 ) {
        if ( req->is_chunked ) {
            char *data = P_VALUE(req->payload.buf);
            int len = (int)req->payload.size;
            int trData = 0;
            while ( len >= 0 ) {
                char buf[6];
                int chunk = len > CHUNK_SIZE ? CHUNK_SIZE : len;
                int ret = sprintf(buf, "%02X\r\n", chunk);
                client_send_direct(cli, buf, ret);
                if ( chunk ) client_send_direct(cli, data + trData, chunk);
                trData += chunk;
                len -= chunk;
                client_send_direct(cli, "\r\n", 2);
                if ( !chunk ) break;
            }
            return 0;
        } else {
          int ret = client_send_direct(cli, P_VALUE(req->payload.buf), req->payload.size);
          return ret;
        }
    }
    return -1;
}

static uint8_t *wait_line(http_client_t *cli) {
  queue_buffer_t *buf = cli->queue;
  uint8_t *crlf = NULL;
  while ( ! (crlf = (uint8_t*)strstr((char*)queue_rd_addr(buf), "\r\n") ) ) {
      if( queue_capacity(buf) > 0 ) {
          int ret = client_recv( cli, LINE_CHUNK );
          if ( ret < 0 ) return NULL;
          HTTP_DBG("Read %d chars; In buf: [%s]", ret, buf->buffer);
      } else {
          return NULL;
      }
  }
  return crlf;
}

static int receive_response(http_client_t *cli, http_response_t *res) {
    queue_clear(cli->queue);
    uint8_t *crlf = wait_line(cli);
    if ( !crlf ) return -1;
    if ( queue_null_terminate(cli->queue, crlf) < 0 ) return -1;
    DBG("resp: {%s}", queue_rd_addr(cli->queue));
    if( sscanf((char*)queue_rd_addr(cli->queue), "HTTP/1.1 %4d", &res->m_httpResponseCode) != 1 ) {
        DBG("Not a correct HTTP answer : %s", queue_rd_addr(cli->queue));
        return -1;
    }
    queue_shift(cli->queue, QUEUE_POS(crlf) + 2);

    DBG("Response code %d", res->m_httpResponseCode);
    cli->response_code = res->m_httpResponseCode;
    return 0;
}

static int receive_headers(http_client_t *cli, http_response_t *res) {
    char *crlf = NULL;
    queue_buffer_t *buf = (queue_buffer_t *)cli->queue;
    queue_shift_clear(&http_queue);
    while( ( crlf = (char*)wait_line(cli) ) ) {
        if ( QUEUE_POS(crlf) == 0 ) {
            HTTP_DBG("Headers read done");
            queue_shift(buf, 2);
            break;
        }

        char key[CHUNK_SIZE>>2];
        char value[CHUNK_SIZE];

        int n = sscanf((char*)queue_rd_addr(buf), "%256[^:]: %256[^\r\n]", key, value);
        if ( n == 2 ) {
            HTTP_DBG("Read header : %s: %s", key, value);
            if( !strcmp(key, "Content-Length") ) {
                sscanf(value, "%8d", (int *)&res->recvContentLength);
            } else if( !strcmp(key, "Transfer-Encoding") ) {
                if( !strcmp(value, "Chunked") || !strcmp(value, "chunked") )
                    res->is_chunked = 1;
            } else if( !strcmp(key, "Content-Type") ) {
                http_response_set_content_type(res, property(value, is_stack));
            } else {
#if defined(HTTP_PARSE_HEADER)
                http_response_add_header(res,
                                         p_stack(key),
                                         p_stack(value));
#endif
            }
            queue_shift_immediately(buf, (uint8_t*)crlf + 2);
        } else {
            DBG("Could not parse header");
            PRTCL_ERR();
        }
    }
    return 0;
}

static char *wait_payload(http_client_t *cli, const char *pattern) {
  const uint32_t chunk = 10;
  char *crlf = NULL;
  while( (crlf = strstr((char*)queue_rd_addr(cli->queue), pattern)) == NULL ) {
    // if there is not enough space in a buffer
    if ( queue_capacity(cli->queue) < chunk ) {
      queue_shift_immediately_by_ind(cli->queue, chunk);
    }
    HTTP_DBG("try to get chunk");
    int ret = client_recv( cli, chunk);
    HTTP_DBG("ret %d", ret);
    if ( ret < 0 ) {
      crlf = NULL;
      break;
    }
  }
  return crlf;
}

static int get_chunked_payload_size(http_client_t *cli, http_response_t *res) {
    // find the \r\n in the payload
    // next string shoud start at HEX chunk size
    SSP_PARAMETER_NOT_USED(res);
    int chunk_len = 0;
    char *crlf = wait_payload(cli, "\r\n");
    if ( !crlf ) return -1; // no \r\n - wrong string
    int ret = sscanf((char*)queue_rd_addr(cli->queue), "%4x\r\n", (unsigned int*)&chunk_len);
    if ( ret != 1 ) {
        // couldn't read a chunk size - fail
        queue_shift_immediately(cli->queue, (uint8_t*)crlf + 2);
        chunk_len = 0;
        return -1;
    }
    HTTP_DBG("detect chunk %d", chunk_len);
    queue_shift(cli->queue, QUEUE_POS(crlf) + 2);
    return chunk_len;
}

static int receive_payload(http_client_t *cli, http_response_t *res) {
    int chunk_len = 0;
    int no_data_error = 0;
    do {
        if ( res->is_chunked ) {
            chunk_len = get_chunked_payload_size(cli, res);
        } else {
            chunk_len = res->recvContentLength;
            DBG("Con-Len %d", res->recvContentLength);
        }
        if ( !chunk_len ) break;
        while ( chunk_len ) {
            uint32_t need_to_read = (chunk_len < CHUNK_SIZE-10) ? chunk_len : CHUNK_SIZE-10;
            HTTP_DBG("need to read %d", need_to_read);
            while ( (int)queue_size(cli->queue) < (int)need_to_read ) {
                HTTP_DBG("get chunk add %d", need_to_read-queue_size(cli->queue));
                int ret = client_recv(cli, need_to_read-queue_size(cli->queue));
                if ( ret < 0 ) {
                    // ret < 0 - error
                    DBG("No data");
                    if ( no_data_error ++ > 2) return -1;
                }
            }
            HTTP_DBG("add payload{%d:s}", need_to_read);//, buf);
            if ( http_response_add_payload(res, p_stack(queue_rd_addr(cli->queue)), need_to_read) < 0 ) {
                queue_clear(cli->queue);
                DBG("Payload is failed");
                return -1;
            }
            if ( queue_size(cli->queue) == need_to_read ) {
                queue_clear(cli->queue);
            } else {
                queue_shift_immediately_by_ind(cli->queue, need_to_read);
            }
            chunk_len -= need_to_read;
            HTTP_DBG("%d %d", chunk_len, need_to_read);
        }
        if ( !res->is_chunked ) { break; }
        else {
          char *crlf = wait_payload(cli, "\r\n");
          if ( crlf ) {
              queue_shift(cli->queue, QUEUE_POS(crlf) + 2);
              queue_shift_clear(cli->queue);
          }
        }
    } while(1);

    HTTP_DBG("body{%s}", P_VALUE(res->payload.buf));
    return 0;
}

int http_client_do(http_client_t *cli, http_request_t *req, http_response_t *res) {
    int ret;
    http_response_init(res, &req->_response_payload_meth);
    if ( cli->sock < 0 ) {
        DBG("new TCP connection");
    	ret = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    	if ( ret < 0 ) return ret;
    	cli->sock = ret;
    	struct sockaddr_in serv;
    	struct hostent *serv_resolve;
      serv_resolve = gethostbyname(P_VALUE(req->host));
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

    	struct timeval tv;
    	tv.tv_sec =     (time_t)        ( cli->timeout / 1000 );
    	tv.tv_usec =    (suseconds_t)   (( cli->timeout % 1000 ) * 1000);
    	setsockopt(cli->sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

    	ret = connect(cli->sock, (struct sockaddr*)&serv, sizeof(serv));
    	if ( ret < 0 ) {
    		DBG("connect fail");
    		soc_close(cli->sock);
    		cli->sock = -1;
    		return -1;
    	}
    	HTTP_DBG("connect done");
    	if ( req->is_cipher ) {
    		if ( ssl_connect(cli->sock) < 0 ) {
    			HTTP_DBG("SSL connect fail");
    			ssl_close(cli->sock);
    			soc_close(cli->sock);
    			cli->sock = -1;
    			return -1;
    		}
    		cli->_r_func = ssl_read;
    		cli->_w_func = ssl_write;
    	} else {
    		cli->_r_func = simple_read;
    		cli->_w_func = simple_write;
    	}
    }

    if ( send_start(cli, req, &http_queue) < 0 ) {
        DBG("send start fail");
        return -1;
    }

    if ( send_header(cli, req, &http_queue) < 0 ) {
        DBG("send header fail");
        return -1;
    }

    if ( !IS_EMPTY(req->payload.buf) ) {
        if ( send_payload(cli, req) < 0 ) {
            DBG("send payload fail");
            return -1;
        }
    }

    HTTP_DBG("Receiving response");

    queue_clear(&http_queue);

    ret = receive_response(cli, res);
    if ( ret < 0 ) {
        DBG("Receiving error (%d)", ret);
        return -1;
    }

    HTTP_DBG("Reading headers");
    res->header = NULL;
    memset(&res->content_type, 0x0, sizeof(http_header_t));
    memset(&res->payload, 0x0, sizeof(http_payload_t));
    res->is_chunked = 0;
    res->processed_payload_chunk = 0;

    //Now let's get a headers
    ret = receive_headers(cli, res);
    if ( ret < 0 ) {
        DBG("Receiving headers error (%d)", ret);
        return -1;
    }

    ret = receive_payload(cli, res);
    if ( ret < 0 ) {
        DBG("Receiving payload error (%d)", ret);
        return -1;
    }
    return 0;
}
