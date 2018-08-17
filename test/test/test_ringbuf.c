#include "unity.h"
#include <sys/mem.h>
#include <debug.h>
#include <data/ringbuffer.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_create_buffer(void) {
    ring_buffer_t rb;
    int res = ringbuf_init(&rb, 512);
    TEST_ASSERT_EQUAL_INT(0, res);
    TEST_ASSERT(rb.buffer);
    TEST_ASSERT_EQUAL_INT(512, rb.total);
    TEST_ASSERT_EQUAL_INT(0, rb.size);
    TEST_ASSERT_EQUAL_INT(0, rb.shift);
    ringbuf_free(&rb);
}

void test_capacity( void ) {
    ring_buffer_t rb;
    int res = ringbuf_init(&rb, 512);
    int cap = ringbuf_capacity(&rb);
    int size = ringbuf_size(&rb);
    TEST_ASSERT_EQUAL_INT( cap, 511 );
    TEST_ASSERT_EQUAL_INT( size, 0 );
    ringbuf_free(&rb);
}


void test_add_simple(void) {
    ring_buffer_t rb;
    int res = ringbuf_init(&rb, 512);
    char *test_buf = "testtesttest";
    int test_buf_len = strlen(test_buf);
    int ret = ringbuf_push(&rb, test_buf, test_buf_len);
    TEST_ASSERT_EQUAL_INT( ret, 0 );
    int size = ringbuf_size(&rb);
    TEST_ASSERT_EQUAL_INT( size, test_buf_len );
    int cap = ringbuf_capacity(&rb);
    TEST_ASSERT_EQUAL_INT( cap, 511 - test_buf_len );
    TEST_ASSERT_EQUAL_INT( rb.shift, 0 );
    TEST_ASSERT_EQUAL_INT( rb.size, test_buf_len );
    TEST_ASSERT_EQUAL_STRING( rb.buffer, test_buf );    
    ringbuf_free(&rb);
}

void test_add_overflow(void) {
    ring_buffer_t rb;
    int res = ringbuf_init(&rb, 10);    
    char *test_buf = "testtesttesttest";
    int test_buf_len = strlen(test_buf);
    int ret = ringbuf_push(&rb, test_buf, test_buf_len);
    TEST_ASSERT_EQUAL_INT( -1, ret );    
    int cap = ringbuf_capacity(&rb);
    int size = ringbuf_size(&rb);
    TEST_ASSERT_EQUAL_INT( cap, 9 );
    TEST_ASSERT_EQUAL_INT( size, 0 );
    ringbuf_free(&rb);
}

void test_pop_simple(void) {
    ring_buffer_t rb;
    int res = ringbuf_init(&rb, 512);
    char *test_buf = "testtesttest";
    int test_buf_len = strlen(test_buf);
    int ret = ringbuf_push(&rb, test_buf, test_buf_len);
    TEST_ASSERT_EQUAL_INT( ret, 0 );
    char buf[1024] = {0};
    ret = ringbuf_pop(&rb, buf, test_buf_len);
    TEST_ASSERT_EQUAL_INT( ret, 0 );
    TEST_ASSERT_EQUAL_STRING( buf, test_buf );
    int size = ringbuf_size(&rb);
    TEST_ASSERT_EQUAL_INT( size, 0 );
    int cap = ringbuf_capacity(&rb);
    TEST_ASSERT_EQUAL_INT( cap, 511 );
    ringbuf_free(&rb);
}

void test_pop_push_complex(void) {
    ring_buffer_t rb;
    char *test_seq  = "0123456789";
    int push_seq[] = { 1, 2, 3, 4, 5, 5, 5 };
    int pop_seq[] = { 0, 0, 3, 2, 4, 6, 5, 5 };
    int res = ringbuf_init(&rb, 20);
    char buf[1024] = {0};
    int i = 0;
    char *tmp = buf;
    for ( i = 0; i<sizeof(pop_seq)/sizeof(int); i++ ) {
        int ret = ringbuf_push(&rb, test_seq, push_seq[i]);
        TEST_ASSERT_EQUAL_INT( 0, ret );
        ret = ringbuf_pop(&rb, tmp, pop_seq[i]);
        tmp += pop_seq[i];
        TEST_ASSERT_EQUAL_INT( 0, ret );
    }
    TEST_ASSERT_EQUAL_STRING( "0010120123012340123401234", buf );
    ringbuf_free(&rb);
}

