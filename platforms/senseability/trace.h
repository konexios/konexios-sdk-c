#ifndef PLATFORMS_SENSEABILITY_TRACE_H_
#define PLATFORMS_SENSEABILITY_TRACE_H_

    
#define DBG_LINE_SIZE 150
    
void dbg_line(const char *fmt, ...);

#define DBG(...) dbg_line(__VA_ARGS__)


#endif