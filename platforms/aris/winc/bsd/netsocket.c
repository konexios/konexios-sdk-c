/*
 * netsocket.c
 *
 *  Created on: 21 окт. 2016 г.
 *      Author: ddemidov
 */

#include <stdlib.h>
#include "wifi_thread.h"
#include "reloc_macro.h"
#include "netsocket.h"
//#include <debug.h>
#include "TRACE_USE.h"
#include <time/time.h>

int net_socket_init(wifi_socket_t *sock, SOCKET curr) {
    char evgroup[15];
    sock->sock = curr;
    sock->next = NULL;
    sock->waitopt = 30000;//TX_WAIT_FOREVER; // FIXME for test
    sock->buf = NULL;
    if ( curr >= 0 ) {
        snprintf(evgroup, 15, "socket_eg_%d", (int)curr); // should be unique
        DBG("server: init flags %s", evgroup);
        SSP_ASSERT( tx_event_flags_create(&sock->flags, evgroup) == TX_SUCCESS );
        SSP_ASSERT( tx_event_flags_set(&sock->flags, 0x00, TX_AND) == TX_SUCCESS );
    }
    return SSP_SUCCESS;
}

wifi_socket_t *net_socket_find(wifi_socket_t *sock, SOCKET curr) {
    wifi_socket_t *s = sock;
    while( s && s->sock != curr ) {
        s = s->next;
    }
    return s;
}

wifi_socket_t *net_socket_add(wifi_socket_t **sock, SOCKET curr) {
    wifi_socket_t *s = *sock;
    if ( s ) {
        while( s->next )  s = s->next;
    }
    wifi_socket_t *tmp = malloc(sizeof(wifi_socket_t));
    if ( !tmp ) {
        DBG("[socket] malloc fail, %d", (int)curr);
        return NULL;
    }
    net_socket_init(tmp, curr);
    if ( s ) s->next = tmp;
    else *sock = tmp;
    return tmp;
}

static void buf_init(net_buffer_t *buf) {
    memset(buf, 0x00, sizeof(net_buffer_t));
}

void buf_free(net_buffer_t *buf) {
    free(buf->buffer);
    free(buf);
}

void net_socket_free(wifi_socket_t **list, SOCKET curr) {
    wifi_socket_t *s = *list;
    wifi_socket_t *prev = NULL;
    while( s && s->sock != curr ) {
        prev = s;
        s = s->next;
    }
    if ( s ) {
        tx_event_flags_delete(&s->flags);
        if ( prev ) prev->next = s->next;
        else *list = s->next;
        net_buffer_t *buf = s->buf;
        while(buf) {
            net_buffer_t *old = buf;
            buf = buf->next;
            buf_free(old);
        }
        free(s);
        s = NULL;
    }
}

int add_buffer(wifi_socket_t *ssock, uint8_t *src, size_t size) {
    net_buffer_t *buf = ssock->buf;
    net_buffer_t *alloc_buf = (net_buffer_t *)malloc(sizeof(net_buffer_t));
    buf_init(alloc_buf);
    if ( buf ) {
        while(buf->next) buf = buf->next;
        buf->next = alloc_buf;
    } else {
        ssock->buf = alloc_buf;
    }
    alloc_buf->buffer = malloc(size);
    alloc_buf->size = size;
    memcpy(alloc_buf->buffer, src, size);
    return 0;
}

int get_buffer(wifi_socket_t *ssock, uint8_t *dst, uint16_t size) {
    net_buffer_t *buf = ssock->buf;
    if ( !buf ) return -1;
    size_t chunk = 0;
    while( chunk < size && buf ) {
        if ( buf->size - buf->start <= size - chunk ) {
            memcpy(dst + chunk, buf->buffer + buf->start, buf->size - buf->start);
            chunk += buf->size - buf->start;
            net_buffer_t *old = buf;
            buf = buf->next;
            buf_free(old);
            ssock->buf = buf;
        } else {
            memcpy(dst + chunk, buf->buffer + buf->start, size - chunk);
            buf->start += size - chunk;
            chunk += size - chunk;
        }
    }
    return chunk;
}

