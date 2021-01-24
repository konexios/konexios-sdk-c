#include "sys/mutex.h"
#include <sys/mem.h>
#include <debug.h>

#if defined(ARROW_THREAD)
int __attribute__((weak)) konexios_mutex_init(konexios_mutex **mutex) {
    SSP_PARAMETER_NOT_USED(mutex);
    DBG("No mutex implementation!");
    return -1;
}

int __attribute__((weak)) konexios_mutex_deinit(konexios_mutex *mutex) {
    SSP_PARAMETER_NOT_USED(mutex);
    DBG("No mutex implementation!");
    return -1;
}

int __attribute__((weak)) konexios_mutex_lock(konexios_mutex *mutex) {
    SSP_PARAMETER_NOT_USED(mutex);
    DBG("No mutex implementation!");
    return -1;
}

int __attribute__((weak)) konexios_mutex_unlock(konexios_mutex *mutex) {
    SSP_PARAMETER_NOT_USED(mutex);
    DBG("No mutex implementation!");
    return -1;
}

#else
typedef void __dummy;
#endif
