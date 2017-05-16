#include "time/watchdog.h"

int __attribute__((weak)) wdt_start()  {
  return 0; // nothing to do
}

int __attribute__((weak)) wdt_feed() {
  return 0; // nothing to do
}

void __attribute__((weak)) wdt_stop() {
  return; // nothing to do
}
