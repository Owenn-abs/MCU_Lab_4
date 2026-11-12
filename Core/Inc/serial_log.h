#ifndef SERIAL_LOG_H
#define SERIAL_LOG_H
#include <stdint.h>
extern volatile uint32_t g_serial_dropped;
void Serial_Log(uint32_t ms, const char *comment);
void Serial_Poll(void);
#endif
