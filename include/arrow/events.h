#if !defined(ARROW_EVENTS_H_)
#define ARROW_EVENTS_H_


typedef enum {
    failed,
    received,
    succeeded
} event_t;


int arrow_send_event_ans(const char *hid, event_t ev, const char *payload);


#endif // ARROW_EVENTS_H_
