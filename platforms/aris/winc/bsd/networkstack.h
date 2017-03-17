/*
 *
 */

#ifndef __NETWORKSTACK_H__
#define __NETWORKSTACK_H__

#include "reloc_macro.h"
#include "reloc_assert.h"
#include "tx_api.h"

#define MAIN_OTA_URL          "http://10.0.0.187/m2m_ota_3a0.bin"

typedef size_t socklen_t;

enum {
    NET_EVENT_DHCP = 1,
    NET_EVENT_STATE_CONNECT     = (1<<1),
    NET_EVENT_STATE_DISCONNECT  = (1<<2),
    NET_EVENT_STATE_DNS         = (1<<3)
};

enum {
    SOCK_CONNECT =  1,
    SOCK_RECV =     (1<<1),
    SOCK_SEND =     (1<<2),
    SOCK_BIND =     (1<<3),
    SOCK_LISTEN =   (1<<4),
    SOCK_ACCEPT =   (1<<5),
    SOCK_TIMEOUT =  (1<<6),
    SOCK_RECVFROM = (1<<7),
    SOCK_SENDTO =   (1<<8),
    SOCK_WAIT_RX =  (1<<9),
    SOCK_WAIT_TX =  (1<<10),
    SOCK_RESERVE =  (1<<11)
};


enum {
    STA,
    AP,
    OTA
};

int wifi_common_init();
int net_sta_init();
int net_ap_init();
int net_ota_init();

void net_set_timeout(uint32_t timeout);
uint32_t net_get_timeout();

INT socket(INT protocol_family, INT socket_type, INT protocol);
void soc_close(INT sock);
//int  net_send(SOCKET sock, uint8_t *buf, uint16_t size);
//int  net_recv(SOCKET sock, uint8_t *buf, uint16_t size);
//
//SOCKET udp_open(const char *host, uint16_t port);
//void udp_close(SOCKET sock);
//int udp_send(SOCKET sock, uint8_t *buf, uint16 size);
//int udp_recv(SOCKET sock, uint8_t *buf, uint16 size);

//int net_socket_free_buffer(SOCKET sock);

//void reserve_socket(SOCKET sock);
//int release_socket(SOCKET sock);


#endif

