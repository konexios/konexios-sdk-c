#ifndef TEST_DNS_FAKE_H_
#define TEST_DNS_FAKE_H_
#include <konexios_config.h>
#include <bsd/socket.h>
#include <stdio.h>

struct hostent *dns_fake(uint32_t ip, const char *host);

#endif
