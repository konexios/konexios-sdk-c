/*
 * networkstack.c
 *
 *  Created on: 12 окт. 2016 г.
 *      Author: ddemidov
 */
#define MODULE_NAME "NET_Stack"
#include "config.h"
#include "networkstack.h"
#include "wifi_thread.h"
#include "driver/include/m2m_wifi.h"
#include "driver/source/nmasic.h"
#include "bus_wrapper/include/nm_bus_wrapper.h"
#include "TRACE_USE.h"
#include "flasharis.h"
#include "wifi_internal_thread.h"
#include "wifi_server_thread.h"
#include "socket/include/socket.h"
#include "_netstack.h"

static NetworkStack net;

static uint8_t wifi_buffer[MAX_BUFFER_SIZE];
static uint8_t udp_buffer[MAX_BUFFER_SIZE];
static uint8_t *cli_buf = wifi_buffer;

void net_set_timeout(uint32_t timeout) {
    net.timeout_ms = timeout;
}

uint32_t net_get_timeout() {
    return net.timeout_ms;
}

static int ts_get_chunk_from_buffer(wifi_socket_t *ssock, uint8_t *dst, uint16_t size) {
    int ret;
    tx_mutex_get( &wifi_mutex, TX_WAIT_FOREVER );
    ret = get_chunk_from_buffer(&ssock->buf, dst, size);
    tx_mutex_put( &wifi_mutex );
    return ret;
}

static int ts_add_chunk_to_buffer(wifi_socket_t *ssock, uint8_t *buf, size_t size) {
    int ret;
    tx_mutex_get( &wifi_mutex, TX_WAIT_FOREVER );
    ret = add_buffer(&ssock->buf, buf, size);
    tx_mutex_put( &wifi_mutex );
    return ret;
}

static int server_sock_init();

static void wifi_event_cb(uint8 u8WiFiEvent, void * pvMsg) {
    switch(u8WiFiEvent) {
        case M2M_WIFI_REQ_DHCP_CONF: {
            tstrM2MIPConfig *pu8IPAddress = (tstrM2MIPConfig*)pvMsg;
            DBG("Associated STA has IP Address \"%u\"\n", (unsigned int)pu8IPAddress->u32StaticIP);
            if (net.mode == AP) {
                if ( server_sock_init() != SSP_SUCCESS ) {
                    DBG("incorrect server sock init");
                }
                tx_event_flags_set(&net.events, NET_EVENT_DHCP, TX_OR);
            } else if ( net.mode == OTA) {
                DBG("start update...");
                m2m_ota_start_update((uint8_t *)MAIN_OTA_URL);
            }
            //            tx_event_flags_set(&net.events, NET_EVENT_DHCP, TX_OR);

        } break;
        case M2M_WIFI_RESP_CON_STATE_CHANGED: {
            tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
            if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
                m2m_wifi_request_dhcp_client();
                tx_event_flags_set(&net.events, NET_EVENT_STATE_CONNECT, TX_OR);
                DBG("wifi connect %d", pstrWifiState->u8ErrCode);

            } else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
                tx_event_flags_set(&net.events, NET_EVENT_STATE_DISCONNECT, TX_OR);
                DBG("wifi disconnect %d", pstrWifiState->u8ErrCode);
                if ( net.mode == AP ) {
                    while ( net.sock ) {
                        DBG("delete %08x", (int)net.sock);
                        close(net.sock->sock);
                        net_socket_free(&net.sock, net.sock->sock);
                    }
                }
            }
        } break;
        default:
            break;
    }
}

static void wifi_socket_cb(SOCKET sock, uint8 u8Msg, void *pvMsg) {
    //    DBG("sock event %d", u8Msg);
    switch(u8Msg) {
        case SOCKET_MSG_CONNECT: {
            tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg*)pvMsg;
            if ( pstrConnect->s8Error == 0 ) {
                wifi_socket_t *ssock = net_socket_find(net.sock, sock);
                if ( ssock ) {
                    if ( tx_event_flags_set(&ssock->flags, SOCK_CONNECT, TX_OR) != TX_SUCCESS ) {
                        TRACE("Cannot set the SOCK_CONNECT flag...\r\n");
                    }
                } else {
                    DBG("connect fail [%d]", sock);
                }
            }
        } break;

        case SOCKET_MSG_RECV: {
            tstrSocketRecvMsg *pstrRecvMsg = (tstrSocketRecvMsg*)pvMsg;
            if ((pstrRecvMsg->pu8Buffer != NULL) && (pstrRecvMsg->s16BufferSize > 0)) {
                wifi_socket_t *ssock = net_socket_find(net.sock, sock);
                if ( ssock ) {
                    if ( pstrRecvMsg->s16BufferSize < 0 ) {
                        // connection error or timeout
                        DBG("recv error %d", pstrRecvMsg->s16BufferSize);
                        tx_event_flags_set(&ssock->flags, SOCK_TIMEOUT, TX_OR);
                        tx_event_flags_set(&ssock->flags, (uint32_t)~SOCK_WAIT_RX, TX_AND);
                    } else {
                        ts_add_chunk_to_buffer(ssock, pstrRecvMsg->pu8Buffer, (size_t)pstrRecvMsg->s16BufferSize);
                        if ( pstrRecvMsg->u16RemainingSize == 0 ) {
                            //                            DBG("received size : %d", ssock->buf.size);
                            tx_event_flags_set(&ssock->flags, (uint32_t)~SOCK_WAIT_RX, TX_AND);
                            tx_event_flags_set(&ssock->flags, SOCK_RECV, TX_OR);
                        }
                    }
                }
            }
        } break;
        case SOCKET_MSG_SENDTO: {
            wifi_socket_t *ssock = net_socket_find(net.sock, sock);
            if ( ssock ) {
                tx_event_flags_set(&ssock->flags, SOCK_SENDTO, TX_OR);
            }
        } break;
        case SOCKET_MSG_RECVFROM: {
            tstrSocketRecvMsg *pstrRecvMsg = (tstrSocketRecvMsg*)pvMsg;
            if ((pstrRecvMsg->pu8Buffer != NULL) && (pstrRecvMsg->s16BufferSize > 0)) {
                wifi_socket_t *ssock = net_socket_find(net.sock, sock);
                if ( ssock ) {
                    ts_add_chunk_to_buffer(ssock, pstrRecvMsg->pu8Buffer, (size_t)pstrRecvMsg->s16BufferSize);
                    if ( pstrRecvMsg->u16RemainingSize == 0 ) {
                        DBG("udp received size : %d", ssock->buf.size);
                        tx_event_flags_set(&ssock->flags, SOCK_RECVFROM, TX_OR);
                    }
                }
            }
        } break;
        case SOCKET_MSG_SEND: {
            sint16 rcnd = *((sint16*)pvMsg);
            //            UNUSED_PARAMETER(rcnd);
            //            DBG("send [%d] %d", sock, rcnd);
            wifi_socket_t *ssock = net_socket_find(net.sock, sock);
            if ( ssock ) {
                if ( rcnd <= 0 ) {
                    DBG("send error %d", rcnd);
                    tx_event_flags_set(&ssock->flags, SOCK_TIMEOUT, TX_OR);
                } else {
                    tx_event_flags_set(&ssock->flags, SOCK_SEND, TX_OR);
                }
                tx_event_flags_set(&ssock->flags, (uint32_t)~SOCK_WAIT_TX, TX_AND);
            }
        } break;

        case SOCKET_MSG_BIND: {
            tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg*)pvMsg;
            if ( pstrBind ) {
                if ( pstrBind->status == 0 ) {
                    wifi_socket_t *ssock = net_socket_find(net.sock, sock);
                    tx_event_flags_set(&ssock->flags, SOCK_BIND, TX_OR);
                } else {
                    close(sock);
                }
            }
        } break;
        case SOCKET_MSG_LISTEN: {
            tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg*)pvMsg;
            if ( pstrListen ) {
                if ( pstrListen->status == 0 ) {
                    wifi_socket_t *ssock = net_socket_find(net.sock, sock);
                    if ( ssock ) {
                        DBG("socket listen %d", sock);
                        tx_event_flags_set(&ssock->flags, SOCK_LISTEN, TX_OR);
                    } else {
                        DBG("cannot listen %d", sock);
                    }
                } else {
                    close(sock);
                }
            }
        } break;
        case SOCKET_MSG_ACCEPT: {
            tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg*)pvMsg;
            if ( pstrAccept->sock >= 0 ) {
                if ( pstrAccept->sock > 5 ) {
                    DBG("too much accepts %d", pstrAccept->sock);
                    close(pstrAccept->sock);
                    return;
                }
                wifi_socket_t *ssock = net_socket_find(net.sock, pstrAccept->sock);
                if ( ssock ) {
                    tx_mutex_get(&wifi_mutex, TX_WAIT_FOREVER);
                    tx_event_flags_set(&ssock->flags, 0x00, TX_AND);
                    ssock->buf.size = 0;
                    tx_mutex_put(&wifi_mutex);
                    DBG("socket reaccept %d", ssock->sock);
                } else {
                    DBG("[sock] add accept %d, %08x", pstrAccept->sock, pstrAccept->strAddr.sin_addr.s_addr);
                    ssock = net_socket_add(&net.sock, pstrAccept->sock);
                    if ( ssock ) {
                        DBG("socket accept %d", ssock->sock);
                        tx_mutex_get(&wifi_mutex, TX_WAIT_FOREVER);
                        net_socket_add_buffer(ssock, cli_buf);
                        tx_mutex_put(&wifi_mutex);
                    } else {
                        DBG("socket accept fail %d", sock);
                    }
                }
                tx_event_flags_set(&ssock->flags, SOCK_ACCEPT, TX_OR);
            } else {
                TRACE("accept fail...\r\n");
            }
        } break;
        default: break;
    }
    return;
}

static void wifi_dns_cb(uint8* pu8DomainName, uint32 u32ServerIP) {
    UNUSED_PARAMETER(pu8DomainName);
    net.dns_resolv = u32ServerIP;
    if ( tx_event_flags_set(&net.events, NET_EVENT_STATE_DNS, TX_OR) != TX_SUCCESS ) {
        TRACE("Cannot set the DNS flag...\r\n");
    }
}

int wifi_common_init() {
    nm_bsp_init (&g_wifi_irq);
    nm_bus_config (&g_sf_spi_device);

    tstrWifiInitParam param;
    memset((uint8_t *)&param, 0x00, sizeof(tstrWifiInitParam));
    param.pfAppWifiCb = wifi_event_cb;
    ASSERT( m2m_wifi_init(&param) == M2M_SUCCESS );

    SSP_ASSERT( tx_event_flags_create(&net.events, "net_event_group") == TX_SUCCESS );
    SSP_ASSERT( tx_event_flags_set(&net.events, 0x00, TX_AND) == TX_SUCCESS );
    net.timeout_ms = TX_WAIT_FOREVER;

    return 0;
}

static int server_sock_init() {
    SOCKET server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        DBG("server: failed to create TCP client socket error!\r\n");
    }

    DBG("server: socket: %d", server_sock);
    if ( net_socket_add(&net.sock, server_sock) == NULL ) {
        TRACE("net_socket_init fail\r\n");
        return 1;
    }

    net_socket_add_buffer(net.sock, wifi_buffer);
    return SSP_SUCCESS;
}

int net_ap_init() {

}

int net_sta_init() {

    uint8_t ssid[M2M_MAX_SSID_LEN];
    uint8_t pass[M2M_MAX_SSID_LEN];
    int sec;
    if ( restore_wifi_setting(ssid, pass, &sec) < 0 ) return -2;

    // start init
    net.sock = NULL;
    net.mode = STA;
    socketInit();
    registerSocketCallback(wifi_socket_cb, wifi_dns_cb);
    memset(&net.strM2MAPConfig, 0x00, sizeof(tstrM2MAPConfig));

    ASSERT(m2m_wifi_connect( ssid, (uint8)strlen(ssid),
                             (uint8)sec, pass,
                             M2M_WIFI_CH_ALL ) == M2M_SUCCESS);


    SSP_ASSERT( wifi_internal_run() == TX_SUCCESS );

    uint32_t state = 0;
    if ( tx_event_flags_get( &net.events,
                             NET_EVENT_STATE_CONNECT|NET_EVENT_STATE_DISCONNECT,
                             TX_OR,
                             &state,
                             TX_WAIT_FOREVER ) != TX_SUCCESS ) goto err_exit;
    if ( state & NET_EVENT_STATE_CONNECT ) return 0;

    err_exit:
    m2m_wifi_disconnect();
    return -1;
}

static void OtaUpdateCb(uint8_t u8OtaUpdateStatusType, uint8_t u8OtaUpdateStatus)
{
    DBG("OtaUpdateCb %d %d", u8OtaUpdateStatusType, u8OtaUpdateStatus);
    if (u8OtaUpdateStatusType == DL_STATUS) {
        if (u8OtaUpdateStatus == OTA_STATUS_SUCSESS) {
            /* Start Host Controller OTA HERE ... Before switching.... */
            DBG("OtaUpdateCb m2m_ota_switch_firmware start.");
            m2m_ota_switch_firmware();
        } else {
            DBG("OtaUpdateCb FAIL u8OtaUpdateStatus %d", u8OtaUpdateStatus);
        }
    } else if (u8OtaUpdateStatusType == SW_STATUS) {
        if (u8OtaUpdateStatus == OTA_STATUS_SUCSESS) {
            DBG("OTA Success. Press reset your board.");
            /* system_reset(); */
        }
    }
}

/**
 * \brief OTA notify callback.
 *
 * OTA notify callback typedef.
 */
static void OtaNotifCb(tstrOtaUpdateInfo *pv)
{
    DBG("OtaNotifCb");
}

int net_ota_init() {
    net.mode = OTA;
    m2m_ota_init(OtaUpdateCb, OtaNotifCb);
    return TX_SUCCESS;
}

SOCKET net_connect(const char *host, uint16_t port) {
    SOCKET sock = -1;
    uint32_t state = 0;
    SSP_ASSERT( tx_event_flags_get( &net.events, NET_EVENT_STATE_CONNECT, TX_AND, &state, net.timeout_ms ) == TX_SUCCESS );
    gethostbyname((uint8*)host);
    SSP_ASSERT( tx_event_flags_get( &net.events,
                                    NET_EVENT_STATE_DNS,
                                    TX_AND_CLEAR,
                                    &state,
                                    TX_WAIT_FOREVER ) == TX_SUCCESS );
    net.server_addr.sin_family = AF_INET;
    net.server_addr.sin_port = _htons(port);
    net.server_addr.sin_addr.s_addr = net.dns_resolv;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) >= 0) {
        wifi_socket_t *ssock = net_socket_add(&net.sock, sock);
        net_socket_add_buffer(ssock, wifi_buffer);
        connect(sock, (struct sockaddr*)(&net.server_addr), sizeof(struct sockaddr_in));
        if ( tx_event_flags_get( &ssock->flags,
                                 SOCK_CONNECT,
                                 TX_AND,
                                 &state,
                                 net.timeout_ms ) != TX_SUCCESS ) {
            TRACE("connect timeout...\r\n");
            return -1;
        }
        return sock;
    } else {
        TRACE("socket fail...\r\n");
    }
    return sock;
}



int net_socket_free_buffer(SOCKET sock) {
    wifi_socket_t *ssock = net_socket_find(net.sock, sock);
    if ( ssock ) {
        net_buffer_init(&ssock->buf, ssock->buf.home);
        return 0;
    }
    return -1;
}


void reserve_socket(SOCKET sock) {
    wifi_socket_t *ssock = net_socket_find(net.sock, sock);
    if ( ssock ) {
        if ( tx_event_flags_set(&ssock->flags, SOCK_RESERVE, TX_OR) != TX_SUCCESS )
            TRACE("Cannot set the SOCK_RESERVE flag...\r\n");
    }
}

int release_socket(SOCKET sock) {
    wifi_socket_t *ssock = net_socket_find(net.sock, sock);
    if ( ssock ) {
        tx_event_flags_set(&ssock->flags, (uint32_t)~SOCK_RESERVE, TX_AND);
        return 0;
    }
    return -1;
}
