/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _COMMON_BSD_SOCKET_H_
#define _COMMON_BSD_SOCKET_H_

#include <unint.h>

#if defined(_ARIS_)
# include <bsd/inet.h>
#include "reloc_macro.h"
#if defined(ETH_MODE)
#  include <nx_api.h>
#  include <nxd_bsd.h>
/** <netdb.h> */
#  include "struct_hostent.h"
void arrow_default_ip_interface_set(NX_IP *ip, NX_PACKET_POOL *pool);
struct hostent* gethostbyname(const char *name);
# else
#  include "DRIVERS/WiFiWINC/winc.h"
#  include "struct_hostent.h"
#  define SO_RCVTIMEO     20   /* Enable receive timeout */
#  define SO_SNDTIMEO     21   /* Enable send timeout */
#  define PF_INET AF_INET
#  define IPPROTO_TCP 0
#  define htons _htons
#  define htonl _htonl
#  define ntohl _ntohl
# endif

#elif defined(__MBED__)
# include "platforms/nucleo/bsd/_socket.h"
STRUCT_HOSTENT
struct hostent* gethostbyname(const char *name);

#elif defined(__linux__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#define soc_close(x) close((x))
    
#elif defined(__XCC__)
# include <qcom/base.h>
# include <qcom_network.h>
# include <qcom/socket_api.h>
# include "struct_hostent.h"
# define socket qcom_socket
# define setsockopt qcom_setsockopt
# define soc_close qcom_socket_close
# define sendto qcom_sendto
# define recvfrom qcom_recvfrom
# define recv qcom_recv
# define send qcom_send
# define connect qcom_connect
struct hostent *gethostbyname(const char *name);

#elif defined(__senseability__)
# include <bsd/inet.h>
# include <platforms/common/bsd/sockdef.h>
# define htons _htons
# define htonl _htonl
# define ntohl _ntohl
STRUCT_HOSTENT

#elif defined(__stm32l475iot__)
# include <bsd/inet.h>
# include <platforms/common/bsd/sockdef.h>
#  include "struct_hostent.h"
# define htons _htons
# define htonl _htonl
# define ntohl _ntohl


#elif defined(__semiconductor__)
# include <platforms/common/bsd/sockdef.h>
# define htons _htons
# ifndef socklen_t
#  define socklen_t uint32_t
# endif
STRUCT_HOSTENT

#endif


#endif // _COMMON_BSD_SOCKET_H_
