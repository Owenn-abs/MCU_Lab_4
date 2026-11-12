#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>

#define SCH_MAX_TASKS 16U
#define SCH_TICK_MS 10U
#define SCH_INVALID_TASK_ID SCH_MAX_TASKS /* handout: first index past the table */

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint8_t RunMe;
    uint32_t TaskID;
} sTask;

typedef enum {
    SCH_OK, SCH_ERROR_CALLBACK, SCH_ERROR_RANGE, SCH_ERROR_FULL,
    SCH_ERROR_ID
} SCH_Error;

/* Init with timer stopped. SCH_Update only increments the timer tick (O(1));
 * the main-loop dispatcher updates Delay/RunMe from absolute deadlines.
 * Delay/Period are ticks. IDs are task-table indexes, as in the handout. */
void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD);
uint8_t SCH_Delete_Task(uint32_t taskID);
uint32_t get_time(void);
SCH_Error SCH_GetLastError(void);
extern volatile uint32_t g_missed_deadline_count;
#endif
