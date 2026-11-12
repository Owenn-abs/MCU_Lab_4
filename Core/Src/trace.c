#include "trace.h"
#include "scheduler.h"
volatile TraceEvent g_trace[TRACE_CAPACITY];
volatile uint32_t g_trace_count;
volatile uint32_t g_10ms_callback_count;

void Trace_Init(void)
{
    uint32_t i;
    g_trace_count = 0U;
    g_10ms_callback_count = 0U;
    for (i = 0U; i < TRACE_CAPACITY; ++i) {
        g_trace[i] = (TraceEvent){0};
    }
}

void Trace_Record(uint16_t event_id, uint16_t aux)
{
    uint32_t index = g_trace_count % TRACE_CAPACITY;
    g_trace[index].timestamp_ms = get_time();
    g_trace[index].event_id = event_id;
    g_trace[index].aux = aux;
    ++g_trace_count; /* Publish only after the event is completely written. */
}
