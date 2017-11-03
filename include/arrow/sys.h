#if !defined(ARROW_SYS_H_)
#define ARROW_SYS_H_

typedef int(*_at_reboot)(void *arg);

void at_reboot(_at_reboot func, void *arg);

void reboot(void);

#endif  // ARROW_SYS_H_
