#include "time/time.h"
#if defined (__XCC__)
#include <qcom/base.h>
#include <platforms/xtensa/time/xcctime.h>
#else
#include <stdio.h>
#endif

time_t build_time() {
  static const char *built = __DATE__" "__TIME__;
  struct tm t;
  const char *ret = (const char *)strptime(built, "%b %d %Y %H:%M:%S", &t);
  if ( ret ) {
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    return mktime(&t);
  }

  return 0;
}

#if !defined(__MBED__)
void set_time(time_t t) {
    stime(&t);
}
#endif

