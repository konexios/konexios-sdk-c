#ifndef _ARROW_KRONOS_C_SDK_XCCTIME_H_
#define _ARROW_KRONOS_C_SDK_XCCTIME_H_

time_t mktime(struct tm *timeptr);
char *strptime (const char *buf, const char *format, struct tm *tm);
int stime(time_t *t);

#endif
