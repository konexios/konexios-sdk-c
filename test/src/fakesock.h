#pragma once
#include <bsd/socket.h>
#include <stdio.h>

struct sockaddr_in *prepsock(struct hostent *fake_addr, short port);
