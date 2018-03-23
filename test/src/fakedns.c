#include "fakedns.h"
#include <string.h>

struct hostent *dns_fake(uint32_t ip, const char *host) {
    static struct hostent s_hostent;
    static char *s_aliases;
    static unsigned long s_hostent_addr;
    static unsigned long *s_phostent_addr[2];

    s_hostent_addr = ip;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = NULL;
    s_hostent.h_name = (char*) host;
    s_hostent.h_aliases = &s_aliases;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(unsigned long);
    s_hostent.h_addr_list = (char**) &s_phostent_addr;
    s_hostent.h_addr = s_hostent.h_addr_list[0];

    return &s_hostent;
}
