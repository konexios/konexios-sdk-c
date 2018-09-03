#include "data/find_by.h"
#include <sys/mem.h>

static const char *find_by_name[] = {
  "userHid",
  "uid",
  "type",
  "gatewayHid",
  "createdBefore",
  "createdAfter",
  "updatedBefore",
  "updatedAfter",
  "enabled",
  "_page",
  "_size",
  "createdDateFrom",
  "createdDateTo",
  "sortField",
  "sortDirection",
  "statuses",
  "systemNames",
  "fromTimestamp",
  "toTimestamp",
  "telemetryNames",
  "osNames"
};

property_t get_find_by_name(int num) {
  if ( num < FindBy_count ) {
    return p_const(find_by_name[num]);
  }
  return p_null;
}

int find_by_validate_key(find_by_t *fb) {
    if ( fb && fb->key < FindBy_count ) return 0;
    return -1;
}
