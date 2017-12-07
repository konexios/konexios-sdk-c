/* AUTOGENERATED FILE. DO NOT EDIT. */

/*=======Test Runner Used To Run Each Test Below=====*/
#define RUN_TEST(TestFunc, TestLineNum) \
{ \
  Unity.CurrentTestName = #TestFunc; \
  Unity.CurrentTestLineNumber = TestLineNum; \
  Unity.NumberOfTests++; \
  if (TEST_PROTECT()) \
  { \
      setUp(); \
      TestFunc(); \
  } \
  if (TEST_PROTECT()) \
  { \
    tearDown(); \
  } \
  UnityConcludeTest(); \
}

/*=======Automagically Detected Files To Include=====*/
#include "unity.h"
#include <setjmp.h>
#include <stdio.h>

int GlobalExpectCount;
int GlobalVerifyOrder;
char* GlobalOrderError;

/*=======External Functions This Runner Calls=====*/
extern void setUp(void);
extern void tearDown(void);
extern void test_create_buffer(void);
extern void test_capacity(void );
extern void test_add_simple(void);
extern void test_add_overflow(void);
extern void test_pop_simple(void);
extern void test_pop_push_complex(void);


/*=======Test Reset Option=====*/
void resetTest(void);
void resetTest(void)
{
  tearDown();
  setUp();
}


/*=======MAIN=====*/
int main(void)
{
  UnityBegin("test_ringbuf.c");
  RUN_TEST(test_create_buffer, 15);
  RUN_TEST(test_capacity, 26);
  RUN_TEST(test_add_simple, 37);
  RUN_TEST(test_add_overflow, 54);
  RUN_TEST(test_pop_simple, 68);
  RUN_TEST(test_pop_push_complex, 86);

  return (UnityEnd());
}
