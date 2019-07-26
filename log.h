#ifndef __LOG_H__
#define __LOG_H__
#include "hal.h"

#define MAX_FILENAME 128
#define MAX_LOG_LEN 1024
#define LOG_OK_LED() palSetPad(GPIOD, GPIOD_LED_GREEN)
#define LOG_WARN_LED() palSetPad(GPIOD, GPIOD_LED_BLUE)
#define LOG_ERR_LED() palSetPad(GPIOD, GPIOD_LED_RED)
#define LOG_CLEAR_LED() palClearPad(GPIOD, GPIOD_LED_RED);palClearPad(GPIOD, GPIOD_LED_BLUE);palClearPad(GPIOD, GPIOD_LED_GREEN)
#define LOG_TIME() TIME_I2MS(chVTGetSystemTime())

#define LOG_FILENAME "out.log"
static const SerialConfig LOG_CONF = {38400, 0, 0, 0};

//Log over SD, serial, or both
#define LOG_SD 0
#define LOG_SERIAL 1


#define LOG_CRITICAL 0
#define LOG_ERR 1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_VERBOSE 4

// If importance <= log_level it will be logged
#define LOG_LEVEL 4


uint8_t log_init(void);
uint8_t log_data(void);
uint8_t log_image(void);
void log_close(void);
void log_message(const char*, uint8_t level);
#endif
