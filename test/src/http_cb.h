#ifndef TEST_HTTP_CB_H_
#define TEST_HTTP_CB_H_
#include <stdio.h>

void set_http_cb(char *buf, int size);

ssize_t send_cb(int sockfd, const void *buf, size_t len, int flags, int count);
ssize_t recv_cb(int sockfd, void *buf, size_t len, int flags, int count);

#endif
