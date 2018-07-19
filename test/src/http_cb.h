#ifndef TEST_HTTP_CB_H_
#define TEST_HTTP_CB_H_
#include <stdio.h>

void set_http_cb(char *buf, int size);
void add_http_cb(char *buf, int size);

ssize_t send_cb(int sockfd, const void *buf, size_t len, int flags, int count);
ssize_t recv_cb(int sockfd, void *buf, size_t len, int flags, int count);
ssize_t sendto_cb(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen, int count);
ssize_t recvfrom_cb(int sock, void *buf, size_t size, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen, int count);


#endif
