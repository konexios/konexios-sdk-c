#include "build/temp/_test_ringbuf.c"
#include "data/ringbuffer.h"
#include "config.h"
#include "unity.h"


void setUp(void)

{

}



void tearDown(void)

{

}



void test_create_buffer(void) {

    ring_buffer_t rb;

    int res = ringbuf_init(&rb, 512);

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((res)), (((void *)0)), (UNITY_UINT)(18), UNITY_DISPLAY_STYLE_INT);

    if ((rb.buffer)) {} else {UnityFail( ((" Expression Evaluated To FALSE")), (UNITY_UINT)((UNITY_UINT)(19)));};

    UnityAssertEqualNumber((UNITY_INT)((512)), (UNITY_INT)((rb.total)), (((void *)0)), (UNITY_UINT)(20), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((rb.size)), (((void *)0)), (UNITY_UINT)(21), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((rb.shift)), (((void *)0)), (UNITY_UINT)(22), UNITY_DISPLAY_STYLE_INT);

    ringbuf_free(&rb);

}



void test_capacity( void ) {

    ring_buffer_t rb;

    int res = ringbuf_init(&rb, 512);

    int cap = ringbuf_capacity(&rb);

    int size = ringbuf_size(&rb);

    UnityAssertEqualNumber((UNITY_INT)((cap)), (UNITY_INT)((511)), (((void *)0)), (UNITY_UINT)(31), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((size)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(32), UNITY_DISPLAY_STYLE_INT);

    ringbuf_free(&rb);

}





void test_add_simple(void) {

    ring_buffer_t rb;

    int res = ringbuf_init(&rb, 512);

    char *test_buf = "testtesttest";

    int test_buf_len = strlen(test_buf);

    int ret = ringbuf_push(&rb, test_buf, test_buf_len);

    UnityAssertEqualNumber((UNITY_INT)((ret)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(43), UNITY_DISPLAY_STYLE_INT);

    int size = ringbuf_size(&rb);

    UnityAssertEqualNumber((UNITY_INT)((size)), (UNITY_INT)((test_buf_len)), (((void *)0)), (UNITY_UINT)(45), UNITY_DISPLAY_STYLE_INT);

    int cap = ringbuf_capacity(&rb);

    UnityAssertEqualNumber((UNITY_INT)((cap)), (UNITY_INT)((511 - test_buf_len)), (((void *)0)), (UNITY_UINT)(47), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((rb.shift)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(48), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((rb.size)), (UNITY_INT)((test_buf_len)), (((void *)0)), (UNITY_UINT)(49), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualString((const char*)((rb.buffer)), (const char*)((test_buf)), (((void *)0)), (UNITY_UINT)(50));

    ringbuf_free(&rb);

}



void test_add_overflow(void) {

    ring_buffer_t rb;

    int res = ringbuf_init(&rb, 10);

    char *test_buf = "testtesttesttest";

    int test_buf_len = strlen(test_buf);

    int ret = ringbuf_push(&rb, test_buf, test_buf_len);

    UnityAssertEqualNumber((UNITY_INT)((-1)), (UNITY_INT)((ret)), (((void *)0)), (UNITY_UINT)(60), UNITY_DISPLAY_STYLE_INT);

    int cap = ringbuf_capacity(&rb);

    int size = ringbuf_size(&rb);

    UnityAssertEqualNumber((UNITY_INT)((cap)), (UNITY_INT)((9)), (((void *)0)), (UNITY_UINT)(63), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((size)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(64), UNITY_DISPLAY_STYLE_INT);

    ringbuf_free(&rb);

}



void test_pop_simple(void) {

    ring_buffer_t rb;

    int res = ringbuf_init(&rb, 512);

    char *test_buf = "testtesttest";

    int test_buf_len = strlen(test_buf);

    int ret = ringbuf_push(&rb, test_buf, test_buf_len);

    UnityAssertEqualNumber((UNITY_INT)((ret)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(74), UNITY_DISPLAY_STYLE_INT);

    char buf[1024] = {0};

    ret = ringbuf_pop(&rb, buf, test_buf_len);

    UnityAssertEqualNumber((UNITY_INT)((ret)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(77), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualString((const char*)((buf)), (const char*)((test_buf)), (((void *)0)), (UNITY_UINT)(78));

    int size = ringbuf_size(&rb);

    UnityAssertEqualNumber((UNITY_INT)((size)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(80), UNITY_DISPLAY_STYLE_INT);

    int cap = ringbuf_capacity(&rb);

    UnityAssertEqualNumber((UNITY_INT)((cap)), (UNITY_INT)((511)), (((void *)0)), (UNITY_UINT)(82), UNITY_DISPLAY_STYLE_INT);

    ringbuf_free(&rb);

}



void test_pop_push_complex(void) {

    ring_buffer_t rb;

    char *test_seq = "0123456789";

    int push_seq[] = { 1, 2, 3, 4, 5, 5, 5 };

    int pop_seq[] = { 0, 0, 3, 2, 4, 6, 5, 5 };

    int res = ringbuf_init(&rb, 20);

    char buf[1024] = {0};

    int i = 0;

    char *tmp = buf;

    for ( i = 0; i<sizeof(pop_seq)/sizeof(int); i++ ) {

        int ret = ringbuf_push(&rb, test_seq, push_seq[i]);

        UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((ret)), (((void *)0)), (UNITY_UINT)(97), UNITY_DISPLAY_STYLE_INT);

        ret = ringbuf_pop(&rb, tmp, pop_seq[i]);

        tmp += pop_seq[i];

        UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((ret)), (((void *)0)), (UNITY_UINT)(100), UNITY_DISPLAY_STYLE_INT);

    }

    UnityAssertEqualString((const char*)(("0010120123012340123401234")), (const char*)((buf)), (((void *)0)), (UNITY_UINT)(102));

    ringbuf_free(&rb);

}
