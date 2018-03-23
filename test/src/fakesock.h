#ifndef TEST_FAKE_SOCKET_H_
#define TEST_FAKE_SOCKET_H_
#include <bsd/socket.h>
#include <stdio.h>

struct sockaddr_in *prepsock(struct hostent *fake_addr, short port);

#endif
