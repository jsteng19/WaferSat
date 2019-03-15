#ifndef __LOG_H__
#define __LOG_H__
#include "hal.h"

#define MAX_FILENAME 128
#define MAX_LOG_LEN 1024
#define LOG_OK_LED() palSetPad(GPIOD, GPIOD_LED_GREEN)
#define LOG_WARN_LED() palSetPad(GPIOD, GPIOD_LED_BLUE)
#define LOG_ERR_LED() palSetPad(GPIOD, GPIOD_LED_RED)
#define LOG_CLEAR_LED() palClearPad(GPIOD, GPIOD_LED_RED);palClearPad(GPIOD, GPIOD_LED_BLUE);palClearPad(GPIOD, GPIOD_LED_GREEN)
#define LOG_CRITICAL() do { palSetPad(GPIOD, GPIOD_LED_RED); chThdSleepMilliseconds(100); LOG_CLEAR_LED(); } while(true)
#define LOG_TIME() TIME_I2S(chVTGetSystemTime())

#define LOG_FILENAME "out.log"

uint8_t log_init(void);
uint8_t log_data(void);

#endif
