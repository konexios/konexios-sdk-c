#include "build/temp/_test_calc.c"
#include "config.h"
#include "mock_rules.h"
#include "calc.h"
#include "unity.h"




void setUp(void)

{

}



void tearDown(void)

{

}



void test_defines(void) {



    UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((0)), (((void *)0)), (UNITY_UINT)(17), UNITY_DISPLAY_STYLE_INT);







}



void test_add( void ) {

    int result = 0;

    rule_is_add_CMockExpectAndReturn(25, 1);

    result = calc_add(2,2);

    UnityAssertEqualNumber((UNITY_INT)((4)), (UNITY_INT)((result)), (((void *)0)), (UNITY_UINT)(27), UNITY_DISPLAY_STYLE_INT);

}



void test_add_fail(void) {

  int result = 0;

  rule_is_add_CMockExpectAndReturn(32, 0);

  result = calc_add(2,2);

  UnityAssertEqualNumber((UNITY_INT)((0)), (UNITY_INT)((result)), (((void *)0)), (UNITY_UINT)(34), UNITY_DISPLAY_STYLE_INT);

}
