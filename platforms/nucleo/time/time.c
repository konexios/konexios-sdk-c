#include "time/time.h"

void get_time(char *ts) {
  struct tm *tmp;
  int ms;
  time_t s = time(NULL);

  tmp = gmtime(&s);
  ms = (us_ticker_read()/1000)%1000;
  sprintf(ts, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", 1900+tmp->tm_year, tmp->tm_mon+1, tmp->tm_mday,
          tmp->tm_hour, tmp->tm_min, tmp->tm_sec, ms);
//  printf("ts: %s\r\n", ts);
}
