/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#ifndef _XTENSA_BSD_SOCKET_H_
#define _XTENSA_BSD_SOCKET_H_

#include <unint.h>
# include <qcom/base.h>
# include <qcom_network.h>
# include <qcom/socket_api.h>
# include <bsd/struct_hostent.h>
# define socket qcom_socket
# define setsockopt qcom_setsockopt
# define soc_close qcom_socket_close
# define sendto qcom_sendto
# define recvfrom qcom_recvfrom
# define recv qcom_recv
# define send qcom_send
# define connect qcom_connect
struct hostent *gethostbyname(const char *name);


#endif // _XTENSA_BSD_SOCKET_H_

