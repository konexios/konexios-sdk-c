#pragma once
#include <config.h>
#include <bsd/socket.h>
#include <stdio.h>

struct hostent *dns_fake(uint32_t ip, const char *host);
