#if !defined(ARROW_TESTSUITE_H_)
#define ARROW_TESTSUITE_H_

#include <arrow/connection.h>

typedef struct _procedure_ {
  property_t hid;
  property_t result_hid;
} test_procedure_t;

#define CREATE_TEST_SUITE(p, hid) \
  test_procedure_t p = { \
    p_const(hid),  \
    p_null() \
  };

int arrow_test_gateway(arrow_gateway_t *gateway, test_procedure_t *proc);
int arrow_test_device(arrow_device_t *device, test_procedure_t *proc);
int arrow_test_begin(test_procedure_t *proc);
int arrow_test_end(test_procedure_t *proc);
int arrow_test_step_begin(test_procedure_t *proc, int number);
int arrow_test_step_end(test_procedure_t *proc, int number, int status, const char *err);
int arrow_test_step_success(test_procedure_t *proc, int number);
int arrow_test_step_fail(test_procedure_t *proc, int number, const char *error);
int arrow_test_step_skip(test_procedure_t *proc, int number);

#endif  // ARROW_TESTSUITE_H_
