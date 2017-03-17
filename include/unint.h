#ifndef KRONOS_C_SDK_UNINT_H_
#define KRONOS_C_SDK_UNINT_H_

#if defined(_ARIS_)
# include <inttypes.h>
#include <sys/types.h>
#elif defined(__MBED__)
# include <inttypes.h>
# include <sys/types.h>
#elif defined(__XCC__)
#include <qcom/basetypes.h>
#include <qcom/stdint.h>
typedef A_INT64 int64_t;
#elif defined(__linux__)
#include <inttypes.h>
#endif
#endif
