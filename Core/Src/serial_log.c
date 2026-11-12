#include "serial_log.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
/* Main-context producer/consumer: no UART interrupt or blocking wait. */
static char queue[2048];
static unsigned head, tail;
volatile uint32_t g_serial_dropped;
void Serial_Log(uint32_t ms, const char *comment)
{
    char line[128];
    int n = snprintf(line, sizeof line, "[%lums] %s at %lu tick\r\n",
                     (unsigned long)ms, comment, (unsigned long)(ms / 10U));
    unsigned i;
    if (n < 0 || (unsigned)n >= sizeof line ||
        (unsigned)n > (tail + sizeof queue - head - 1U) % sizeof queue) {
        ++g_serial_dropped;
        return;
    }
    for (i = 0; i < (unsigned)n; ++i) {
        queue[head] = line[i];
        head = (head + 1U) % sizeof queue;
    }
}
void Serial_Poll(void)
{
    if (tail != head && (USART1->SR & USART_SR_TXE)) {
        USART1->DR = (uint8_t)queue[tail];
        tail = (tail + 1U) % sizeof queue;
    }
}
