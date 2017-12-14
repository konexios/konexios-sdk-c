/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#if !defined(KRONOS_C_SDK_BSD_INET_H_) && !defined(USER_BYTE_CONVERTER)
#define KRONOS_C_SDK_BSD_INET_H_

#include <sys/type.h>

uint16_t ntohs(uint16_t n) __attribute__((unused));

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
uint16_t htons(uint16_t n);

/**
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
uint16_t ntohs(uint16_t n);

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
uint32_t htonl(uint32_t n);

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
uint32_t ntohl(uint32_t n);


#endif  // KRONOS_C_SDK_BSD_INET_H_

