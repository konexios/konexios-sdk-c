#if !defined(KONEXIOS_TESTSUITE_H_)
#define KONEXIOS_TESTSUITE_H_

#include <konexios/gateway.h>
#include <konexios/device.h>

// start the gateway test
int konexios_test_gateway(konexios_gateway_t *gateway, property_t *res_hid);
int konexios_test_gateway_proc_hid(konexios_gateway_t *gateway, const char *phid, property_t *res_hid);
int konexios_test_gateway_proc_name(konexios_gateway_t *gateway, const char *name, property_t *res_hid);
// start the device test
int konexios_test_device(konexios_device_t *device, property_t *res_hid);
int konexios_test_device_proc_hid(konexios_device_t *device, const char *phid, property_t *res_hid);
int konexios_test_device_proc_name(konexios_device_t *device, const char *name, property_t *res_hid);
// start the test case
int konexios_test_begin(property_t *res_hid);
// try to finish this test case
int konexios_test_end(property_t *res_hid);
// start current test step
int konexios_test_step_begin(property_t *res_hid, int number);
// notify that test step successed
int konexios_test_step_success(property_t *res_hid, int number);
// notify that test step failed
int konexios_test_step_fail(property_t *res_hid, int number, const char *error);
// notify that test step skipped
int konexios_test_step_skip(property_t *res_hid, int number);

#endif  // KONEXIOS_TESTSUITE_H_
