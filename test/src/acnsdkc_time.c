#include <time/time.h>
#include <sys/mem.h>
#include <unistd.h>


int msleep(int m_sec) {
    usleep(m_sec * 1000);
    return 0;
}

int stime(const time_t *t) {
    return 0;
}
