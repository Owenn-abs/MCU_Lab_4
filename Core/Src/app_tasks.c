#include "app_tasks.h"
#include "main.h"
#include "scheduler.h"
#include "serial_log.h"
#include "trace.h"

volatile uint32_t g_app_task_count[5];
volatile uint32_t g_oneshot_count[2];
volatile uint32_t g_cancelled_task_count;
volatile uint8_t g_delete_test_passed;
static uint32_t cancelled_id;

void Task_Blink_Red(void)
{
    HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
    ++g_app_task_count[0];
    Trace_Record(TRACE_LED_RED, 0U);
    Serial_Log(get_time(), "Task 1 RED toggle");
}
void Task_Blink_Yellow(void)
{
    HAL_GPIO_TogglePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin);
    ++g_app_task_count[1];
    Trace_Record(TRACE_LED_YELLOW, 0U);
    Serial_Log(get_time(), "Task 2 YELLOW toggle");
}
void Task_Blink_Green(void)
{
    HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    ++g_app_task_count[2];
    Trace_Record(TRACE_LED_GREEN, 0U);
    Serial_Log(get_time(), "Task 3 GREEN toggle");
}
void Task_Blink_Blue(void)
{
    HAL_GPIO_TogglePin(GPIOA, BLUE_LED_Pin);
    Serial_Log(get_time(), "Task 4 BLUE toggle");
    ++g_app_task_count[3];
    Trace_Record(TRACE_UP, 0U);
}
void Task_Blink_Pink(void)
{
    HAL_GPIO_TogglePin(GPIOA, PINK_LED_Pin);
    Serial_Log(get_time(), "Task 5 PINK toggle");
    ++g_app_task_count[4];
    Trace_Record(TRACE_DOWN, 0U);
}
void Task_Trace_10ms(void)
{
    ++g_10ms_callback_count;
    Trace_Record(TRACE_TICK_CALLBACK, 0U);
    Serial_Log(get_time(), "10ms callback");
#if LAB_SCOPE_PINS
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
#endif
}
void Task_Once_350ms(void)
{
    ++g_oneshot_count[0];
    Trace_Record(TRACE_ONESHOT_350, 0U);
    Serial_Log(get_time(), "One-shot 350ms");
}
void Task_Once_750ms(void)
{
    ++g_oneshot_count[1];
    Trace_Record(TRACE_ONESHOT_750, 0U);
    Serial_Log(get_time(), "One-shot 750ms");
}
void Task_Cancelled(void)
{
    ++g_cancelled_task_count;
    Trace_Record(TRACE_CANCELLED_RAN, 0U);
}
void Task_Delete_600ms(void)
{
    uint8_t first = SCH_Delete_Task(cancelled_id);
    uint8_t second = SCH_Delete_Task(cancelled_id);
    g_delete_test_passed = (uint8_t)(first == 1U && second == 0U);
    Trace_Record(TRACE_DELETE_TEST, g_delete_test_passed);
    Serial_Log(get_time(), g_delete_test_passed ? "Delete test PASS" : "Delete test FAIL");
}

void App_SetCancelledTaskID(uint32_t id)
{
    cancelled_id = id;
}

void App_Init(void)
{
    uint32_t i;
    Trace_Init();
    g_oneshot_count[0] = g_oneshot_count[1] = 0U;
    g_cancelled_task_count = 0U;
    g_delete_test_passed = 0U;
    HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin |
                     BLUE_LED_Pin | PINK_LED_Pin, GPIO_PIN_SET);
    for (i = 0U; i < 5U; ++i) g_app_task_count[i] = 0U;
}

