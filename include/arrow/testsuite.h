#if !defined(ARROW_TESTSUITE_H_)
#define ARROW_TESTSUITE_H_
#if 0
#include <arrow/connection.h>

int arrow_test_gateway(arrow_gateway_t *gateway);
int arrow_test_device(arrow_device_t *device);
int arrow_test_begin(const char *hid);
int arrow_test_step_begin(const char *hid, int number);
int arrow_test_step_end(const char *hid, int number);
int arrow_test_step_skip(const char *hid, int number);
int arrow_test_end(const char *hid);
#endif
#endif  // ARROW_TESTSUITE_H_
