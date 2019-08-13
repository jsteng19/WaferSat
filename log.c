/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Modified by varun-iyer for ChibiOS/MCU usage
 *
 *
 */

#include "hal.h"
#include "string.h"
#include "sd.h"
#include "chprintf.h"
#include "collector.h"
#include "ov5640.h"
#include "gps.h"
#include "ch.h"
#include "log.h"
#include "ff.h"

static struct {
	mutex_t mtx;
	char log_dirname[MAX_FILENAME];
	FIL fp;
	int level;
} L;


static const char *level_names[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

void log_init(void) {
	chMtxObjectInit(&(L.mtx));
#if LOG_MEM
	// TODO name log with gps date
	FRESULT mk_err = 1;
	int test_no = 0;
	// Find unique directory
	while(mk_err && test_no < 100000) {
		chsnprintf(L.log_dirname, MAX_FILENAME, "test%d", test_no);
		mk_err = f_mkdir(L.log_dirname);
		test_no++;
	}
	if(mk_err != 0) {
		LED_ERR();
		// return mk_err;
	}
	
	char log_filename[MAX_FILENAME];
	chsnprintf(log_filename, MAX_FILENAME, "%s/%s", L.log_dirname, LOG_FILENAME);
 
	FRESULT f_err;
	f_err = f_open(&(L.fp), log_filename, FA_CREATE_NEW);
	if(f_err) {
		LED_ERR();
		//TODO error handling system
		// return f_err;
	}
	f_err = f_close(&(L.fp));
	if(f_err) {
		LED_ERR();
		// return f_err;
	}
	f_err = f_open(&(L.fp), log_filename, FA_WRITE);
	if(f_err) {
		LED_ERR();
		// return f_err;
	}
#endif /* LOG_MEM */
#if LOG_SERIAL
	sdStart(&LOG_SD, &LOG_CFG);
#endif /* LOG_SERIAL */

	log_set_level(LOG_LEVEL);
}

 
void log_set_level(int level) {
	L.level = level;
}
 
void log_data(void) {
	dataPoint_t dp;
	getSensors(&dp);
	 
	long int ms = log_ms();
	long int s = (ms / 1000) % 60;
	long int m = (ms / (60 * 1000)) % 60;
	long int h = ms / (3600 * 1000);
	ms = ms % 1000;
	 
	char log_buf[GPS_MSG_SIZE]; 
	gps_data_t gps = gps_get();
	gps_data_str(log_buf, GPS_MSG_SIZE, &gps);
	 
#if LOG_MEM
	f_printf(&(L.fp), "%li:%02li:%02li.%03li DATA:\r\n", h, m, s, ms);
	f_printf(&(L.fp), "\tBME280 pressure:%d humidity:%d temperature:%d\r\n", dp.sen_i1_press, dp.sen_i1_hum, dp.sen_i1_temp);
	f_printf(&(L.fp), "\tLTR329 ltr329_intensity_ch0:%d ltr329_intensity_ch1:%d\r\n", dp.ltr329_intensity_ch0, dp.ltr329_intensity_ch1);
	f_printf(&(L.fp), "\tSTM32 temp:%d adc_vbat:%d\r\n", dp.stm32_temp, dp.adc_vbat);
	f_printf(&(L.fp), "\tTMP100 temp_0:%d temp_1:%d\r\n", dp.tmp100_0_temp, dp.tmp100_1_temp);
	f_printf(&(L.fp), "\tMPU9250 x:%d y:%d z:%d\r\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
	f_printf(&(L.fp), "\tMPU9250 x:%d y:%d z:%d\r\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
	 
	f_printf(&(L.fp), "\t");
	f_printf(&(L.fp), log_buf);
	f_printf(&(L.fp), "\r\n");
	f_sync(&(L.fp));
#endif /* LOG_SD */
 
#if LOG_SERIAL
	chprintf((BaseSequentialStream*) &LOG_SD, "%li:%02li:%02li.%03li DATA:\r\n", h, m, s, ms);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tBME280 pressure:%d humidity:%d temperature:%d\r\n", dp.sen_i1_press, dp.sen_i1_hum, dp.sen_i1_temp);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tLTR329 ltr329_intensity_ch0:%d ltr329_intensity_ch1:%d\r\n", dp.ltr329_intensity_ch0, dp.ltr329_intensity_ch1);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tSTM32 temp:%d adc_vbat:%d\r\n", dp.stm32_temp, dp.adc_vbat);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tTMP100 temp_0:%d temp_1:%d\r\n", dp.tmp100_0_temp, dp.tmp100_1_temp);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tMPU9250 x:%d y:%d z:%d\r\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
	chprintf((BaseSequentialStream*) &LOG_SD, "\tMPU9250 x:%d y:%d z:%d\r\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
	 
	chprintf((BaseSequentialStream*) &LOG_SD, "\t");
	chprintf((BaseSequentialStream*) &LOG_SD, log_buf);
	chprintf((BaseSequentialStream*) &LOG_SD, "\r\n");
#endif/* LOG_SERIAL */
}
 
void log_image(void) {
	uint32_t time_s = log_ms()/1000;
	char image_filename[MAX_FILENAME];
	chsnprintf(image_filename, MAX_FILENAME, "%s/img%d.jpg", L.log_dirname, time_s);
	uint32_t err = OV5640_Snapshot2SD(image_filename);
	if(err == FR_EXIST) {
		int img_num = 0;
		while(err == FR_EXIST && img_num++ < 1000) {
			chsnprintf(image_filename, MAX_FILENAME, "%s/img%d_%d.jpg", L.log_dirname, time_s, img_num);
			err = OV5640_Snapshot2SD(image_filename);
		}
	}
	return;
}
  
void log_log(int level, const char *file, int line, const char *fmt, ...) {
  /* Acquire lock */
	chMtxLock(&(L.mtx));

  /* Get current time */

  /* Log to stderr */
	long int ms = log_ms();
	long int s = (ms / 1000) % 60;
	long int m = (ms / (60 * 1000)) % 60;
	long int h = ms / (3600 * 1000);
	ms = ms % 1000;
	va_list args;
	 
#if LOG_SERIAL
	if (level >= L.level) {
#if LOG_USE_COLOR
		chprintf(
			(BaseSequentialStream*) &SD1, "%li:%02li:%02li.%03li\t%-5s\x1b[0m \x1b[90m%\ts:%d:\x1b[0m\t",
			h, m, s, ms, level_colors[level], level_names[level], file, line);
#else /* !LOG_USE_COLOR */
		chprintf((BaseSequentialStream*) &SD1, "%li:%02li:%02li.%03li\t%-5s\t%s:%d:\t", h, m, s, ms, level_names[level], file, line);
#endif /* LOG_USE_COLOR */
		va_start(args, fmt);
		chvprintf((BaseSequentialStream*) &SD1, fmt, args);
		va_end(args);
		chprintf((BaseSequentialStream*) &SD1, "\n\r");
	}
#endif /* LOG_SERIAL */

  /* Log to file */
#if LOG_MEM
    f_printf(&(L.fp), "%li:%02li:%02li.%03li\t%-5s\t%s:%d:\t", h, m, s, ms, level_names[level], file, line);
    va_start(args, fmt);
    f_vprintf(&(L.fp), fmt, args);
    va_end(args);
    f_printf(&(L.fp), "\n");
    f_sync(&(L.fp));
#endif /* LOG_MEM */

  /* Release lock */
	chMtxUnlock(&(L.mtx));
}
