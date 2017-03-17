#ifndef ARROW_UTF8_H_
#define ARROW_UTF8_H_

int utf8check(const char *s);
void fix_urldecode(char *query);
void urlencode(char *dst, char *src, int len);

#endif
