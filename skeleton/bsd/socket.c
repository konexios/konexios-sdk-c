/* Copyright (c) 2017 Arrow Electronics, Inc.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License 2.0
 * which accompanies this distribution, and is available at
 * http://apache.org/licenses/LICENSE-2.0
 * Contributors: Arrow Electronics, Inc.
 */

#include "bsd/socket.h"

struct hostent* gethostbyname(const char *name) {
  static struct hostent s_hostent;
  // FIXME implementation
  return &s_hostent;
}

int socket(int protocol_family, int socket_type, int protocol) {
  // FIXME implementation
  return -1;
}

void soc_close(int socket) {
  // FIXME implementation
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
  // FIXME implementation
  return -1;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
  // FIXME implementation
  return -1;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
  // FIXME implementation
  return -1;
}

ssize_t recvfrom(int sock, void *buf, size_t size, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen) {
  // FIXME implementation
  return -1;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  // FIXME implementation
  return -1;
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen) {
  // FIXME implementation
  return -1;
}

int bind(int sock, const struct sockaddr *addr, socklen_t addrlen) {
  // FIXME implementation
  return -1;
}

int listen(int sock, int backlog) {
  // FIXME implementation
  return -1;
}

int accept(int sock, struct sockaddr *addr, socklen_t *addrlen) {
  // FIXME implementation
  return -1;
}
