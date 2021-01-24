#include "unity.h"
#include <debug.h>
#include <sys/mem.h>
#include <data/static_buf.h>
#include <data/static_alloc.h>
#include <konexios/utf8.h>
#include <data/linkedlist.h>
#include <data/property.h>
#include <data/property_base.h>
#include <data/property_const.h>
#include <data/property_dynamic.h>
#include <data/property_stack.h>
#include "socket_weak.h"
#include <bsd/socket.h>

#include <time/time.h>
#include "acnsdkc_time.h"

#include <ntp/ntp.h>
#include "ntp_client.h"
#include "mock_watchdog.h"
#include "mock_sockdecl.h"

#include "http_cb.h"
#include "fakedns.h"
#include "fakesock.h"

void setUp(void) {
    property_types_init();
}

void tearDown(void) {
    property_types_deinit();
}

char ntp_text[] = { 0x24, 0x02, 0x03, 0xe9, 0x00, 0x00, 0x00, 0x2d,
                    0x00, 0x00, 0x09, 0x39, 0xd4, 0xd7, 0x01, 0x9d,
                    0xde, 0xfa, 0x2c, 0x9f, 0x26, 0xce, 0x5a, 0x89,
                    0xde, 0xfa, 0x31, 0x17, 0x35, 0xba, 0xfc, 0x08,
                    0xde, 0xfa, 0x31, 0x17, 0x4a, 0x8b, 0xa1, 0xa8,
                    0xde, 0xfa, 0x31, 0x17, 0x4a, 0x8f, 0x3f, 0x39 };

void test_ntp(void) {
    struct hostent *fake_addr = dns_fake(0xc0a80001, NTP_DEFAULT_SERVER);
    set_http_cb(ntp_text,
                sizeof(ntp_text));
    wdt_feed_IgnoreAndReturn(0);
    socket_ExpectAndReturn(PF_INET, SOCK_DGRAM, 0, 0);
    setsockopt_IgnoreAndReturn(0);
    gethostbyname_ExpectAndReturn(NTP_DEFAULT_SERVER, fake_addr);

    sendto_StubWithCallback(sendto_cb);
    recvfrom_StubWithCallback(recvfrom_cb);
    soc_close_Expect(0);

    int ret = ntp_set_time_common(NTP_DEFAULT_SERVER,
                                  NTP_DEFAULT_PORT,
                                  NTP_DEFAULT_TIMEOUT,
                                  0);

    TEST_ASSERT_EQUAL_INT( 0, ret );
}
