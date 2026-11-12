#ifndef TRACE_H
#define TRACE_H
#include <stdint.h>
#ifndef LAB_SCOPE_PINS
#define LAB_SCOPE_PINS 0
#endif
#define TRACE_CAPACITY 256U
typedef enum {
    TRACE_TICK_CALLBACK = 1, TRACE_LED_RED, TRACE_LED_YELLOW, TRACE_LED_GREEN,
    TRACE_UP, TRACE_DOWN, TRACE_ONESHOT_350, TRACE_ONESHOT_750,
    TRACE_DELETE_TEST, TRACE_CANCELLED_RAN
} TraceEventID;
typedef struct {
    uint32_t timestamp_ms;
    uint16_t event_id;
    uint16_t aux;
} TraceEvent;
extern volatile TraceEvent g_trace[TRACE_CAPACITY];
extern volatile uint32_t g_trace_count;
extern volatile uint32_t g_10ms_callback_count;
void Trace_Init(void);
void Trace_Record(uint16_t event_id, uint16_t aux);
#endif
