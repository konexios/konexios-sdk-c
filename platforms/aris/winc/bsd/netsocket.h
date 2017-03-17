/*
 * netsocket.h
 *
 *  Created on: 21 окт. 2016 г.
 *      Author: ddemidov
 */

#ifndef NETSOCKET_H_
#define NETSOCKET_H_

#include "_netstack.h"

#define MAX_BUFFER_SIZE 4096

int add_buffer(wifi_socket_t *ssock, uint8_t *src, size_t size);
int get_buffer(wifi_socket_t *ssock, uint8_t *dst, uint16_t size);

int net_socket_init(wifi_socket_t *sock, SOCKET curr);
wifi_socket_t *net_socket_find(wifi_socket_t *sock, SOCKET curr);
wifi_socket_t *net_socket_add(wifi_socket_t **sock, SOCKET curr);
void net_socket_free(wifi_socket_t **list, SOCKET curr);
void net_socket_add_buffer(wifi_socket_t *sock, uint8_t *home);

#endif /* NETSOCKET_H_ */
