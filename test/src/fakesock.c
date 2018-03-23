#include "fakesock.h"
#include <string.h>

struct sockaddr_in *prepsock(struct hostent *fake_addr, short port) {
    static struct sockaddr_in serv;
    memset(&serv, 0, sizeof(struct sockaddr_in));
    serv.sin_family = PF_INET;
    bcopy((char *)fake_addr->h_addr,
            (char *)&serv.sin_addr.s_addr,
            (uint32_t)fake_addr->h_length);
    serv.sin_port = htons(port);
    return &serv;
}
