#include "scheduler.h"
#include <stddef.h>
#if defined(STM32F103x6)
#include "stm32f1xx.h"
static uint32_t enter_critical(void) { uint32_t s = __get_PRIMASK(); __disable_irq(); return s; }
static void leave_critical(uint32_t s) { __set_PRIMASK(s); }
#else
static uint32_t enter_critical(void) { return 0U; }
static void leave_critical(uint32_t s) { (void)s; }
#endif

static sTask tasks[SCH_MAX_TASKS];
/* Keep the PDF's sTask layout; deadlines are maintained in main context. */
static uint32_t deadlines[SCH_MAX_TASKS];
static volatile uint32_t scheduler_ticks;
static SCH_Error last_error;
static uint8_t dispatching;
volatile uint32_t g_missed_deadline_count;

void SCH_Init(void)
{
    uint32_t i;
    for (i = 0; i < SCH_MAX_TASKS; ++i) {
        tasks[i] = (sTask){0};
        deadlines[i] = 0U;
    }
    scheduler_ticks = 0U;
    last_error = SCH_OK;
    dispatching = 0U;
    g_missed_deadline_count = 0U;
}

void SCH_Update(void)
{
    ++scheduler_ticks;
}

uint32_t get_time(void)
{
    return scheduler_ticks * SCH_TICK_MS;
}

SCH_Error SCH_GetLastError(void)
{
    return last_error;
}

uint32_t SCH_Add_Task(void (*pFunction)(), uint32_t DELAY, uint32_t PERIOD)
{
    uint32_t i, state = enter_critical();
    if (pFunction == NULL) {
        last_error = SCH_ERROR_CALLBACK;
    } else if (DELAY > INT32_MAX || PERIOD > INT32_MAX) {
        last_error = SCH_ERROR_RANGE;
    } else {
        for (i = 0U; i < SCH_MAX_TASKS; ++i) {
            if (tasks[i].pTask == NULL) {
                tasks[i] = (sTask){pFunction, DELAY, PERIOD, 0U, i};
                deadlines[i] = scheduler_ticks + DELAY;
                last_error = SCH_OK;
                leave_critical(state);
                return i;
            }
        }
        last_error = SCH_ERROR_FULL;
    }
    leave_critical(state);
    return SCH_INVALID_TASK_ID;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    uint32_t state = enter_critical();
    if (taskID < SCH_MAX_TASKS && tasks[taskID].pTask != NULL) {
        tasks[taskID] = (sTask){0};
        deadlines[taskID] = 0U;
        last_error = SCH_OK;
        leave_critical(state);
        return 1U;
    }
    last_error = SCH_ERROR_ID;
    leave_critical(state);
    return 0U;
}

void SCH_Dispatch_Tasks(void)
{
    uint32_t budget, cutoff;
    if (dispatching) {
        return; /* A callback must not recursively dispatch other callbacks. */
    }
    dispatching = 1U;
    cutoff = scheduler_ticks;
    /* Budget bounds callbacks that continually register new tasks. */
    for (budget = 0U; budget < SCH_MAX_TASKS; ++budget) {
        uint32_t i, selected = SCH_MAX_TASKS, oldest_age = 0U;
        uint32_t state = enter_critical();
        uint32_t now = cutoff;
        void (*callback)(void);
        for (i = 0U; i < SCH_MAX_TASKS; ++i) {
            uint32_t age = now - deadlines[i];
            if (tasks[i].pTask == NULL || age > INT32_MAX) {
                if (tasks[i].pTask != NULL) tasks[i].Delay = deadlines[i] - now;
                continue;
            }
            tasks[i].Delay = 0U;
            tasks[i].RunMe = 1U;
            if (selected == SCH_MAX_TASKS || age > oldest_age ||
                (age == oldest_age && tasks[i].TaskID < tasks[selected].TaskID)) {
                selected = i;
                oldest_age = age;
            }
        }
        if (selected == SCH_MAX_TASKS) {
            leave_critical(state);
            break;
        }
        callback = tasks[selected].pTask;
        if (tasks[selected].Period == 0U) {
            tasks[selected] = (sTask){0};
            deadlines[selected] = 0U;
        } else {
            uint32_t skipped = oldest_age / tasks[selected].Period;
            deadlines[selected] += (skipped + 1U) * tasks[selected].Period;
            tasks[selected].Delay = deadlines[selected] - now;
            tasks[selected].RunMe = 0U;
            g_missed_deadline_count += skipped;
        }
        /* No writes to the selected record after invoking: callback may delete
         * itself or another task, and reuse any freed slot. */
        leave_critical(state);
        callback();
    }
    dispatching = 0U;
}
