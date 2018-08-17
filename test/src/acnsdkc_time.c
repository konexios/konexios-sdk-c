#include <time/time.h>
#include <sys/mem.h>
#include <unistd.h>


void get_time(char *ts) {
  struct tm *tmp;
  int ms;
  time_t s = time(NULL);

  struct timeval tv;
  gettimeofday(&tv, NULL);

  tmp = gmtime(&s);
  ms = (tv.tv_usec/1000)%1000;
  sprintf(ts, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", 1900+tmp->tm_year, tmp->tm_mon+1, tmp->tm_mday,
          tmp->tm_hour, tmp->tm_min, tmp->tm_sec, ms);
}

int msleep(int m_sec) {
    usleep(m_sec * 1000);
    return 0;
}

void timestamp(timestamp_t *ts) {
    struct tm *tmp;
    int ms;
    time_t s = time(NULL);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tmp = gmtime(&s);
    ms = (tv.tv_usec/1000)%1000;
    ts->year = 1900 + tmp->tm_year;
    ts->mon = 1 + tmp->tm_mon;
    ts->day = tmp->tm_mday;
    ts->hour = tmp->tm_hour;
    ts->min = tmp->tm_min;
    ts->sec = tmp->tm_sec;
    ts->msec = ms;
}

int stime(const time_t *t) {
    return 0;
}
