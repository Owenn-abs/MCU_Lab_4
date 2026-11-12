#ifndef APP_TASKS_H
#define APP_TASKS_H
#include <stdint.h>
extern volatile uint32_t g_app_task_count[5];
extern volatile uint32_t g_oneshot_count[2];
extern volatile uint32_t g_cancelled_task_count;
extern volatile uint8_t g_delete_test_passed;
void App_Init(void);
void App_SetCancelledTaskID(uint32_t id);
void Task_Blink_Red(void);
void Task_Blink_Yellow(void);
void Task_Blink_Green(void);
void Task_Blink_Blue(void);
void Task_Blink_Pink(void);
void Task_Trace_10ms(void);
void Task_Once_350ms(void);
void Task_Once_750ms(void);
void Task_Cancelled(void);
void Task_Delete_600ms(void);
#endif

