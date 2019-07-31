#include "sd.h"
#include "hal.h"
#include "string.h"
#include "chprintf.h"
#include "ov5640.h"
#include "ff.h"
#include "collector.h"
#include "log.h"

FIL log_file;
char log_dirname[MAX_FILENAME]; 
const char* const log_level_string[] = {"CRITICAL", "ERROR", "WARNING", "INFO", "VERBOSE"};

uint8_t log_init(void) {
	/** 
	* Initializes the log file. 
	* @return 0 if success; a FATFS error otherwise
	**/
#if LOG_SD
	FRESULT mk_err = 1;
	int test_no = 0;
	// Find unique directory
	while(mk_err && test_no < 100000) {
		chsnprintf(log_dirname, MAX_FILENAME, "test%d", test_no);
		mk_err = f_mkdir(log_dirname);
		test_no++;
	}
	if(mk_err != 0) {
		LOG_ERR_LED();
		return mk_err;
	}
	
	char log_filename[MAX_FILENAME];
	chsnprintf(log_filename, MAX_FILENAME, "%s/%s", log_dirname, LOG_FILENAME);
 
	FRESULT f_err;
	f_err = f_open(&log_file, log_filename, FA_CREATE_NEW);
	// TODO don't return if you can handle the error and still log over serial
	if(f_err) {
		LOG_ERR_LED();
		return f_err;
	}
	f_err = f_close(&log_file);
	if(f_err) {
		LOG_ERR_LED();
		return f_err;
	}
	f_err = f_open(&log_file, log_filename, FA_WRITE);
	if(f_err) {
		LOG_ERR_LED();
		return f_err;
	}
#endif /* LOG_SD */
#if LOG_SERIAL
	sdStart(&SD1, &LOG_CONF);
#endif /* LOG_SERIAL */
	return 0;	
}

uint8_t log_data(void) {
	dataPoint_t dp;
	getSensors(&dp);
	uint32_t time = LOG_TIME();
#if LOG_SD
	char log_buf[MAX_LOG_LEN]; 
	char log_buf[MAX_LOG_LEN]; 
	chsnprintf(log_buf, MAX_LOG_LEN, "%d DATA:\r\n", time);
	unsigned int written;
	unsigned int to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	 
	chsnprintf(log_buf, MAX_LOG_LEN, "\tBME280 pressure:%d humidity:%d temperature:%d\n", dp.sen_i1_press, dp.sen_i1_hum, dp.sen_i1_temp);
	written = 0;
	to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	 
	chsnprintf(log_buf, MAX_LOG_LEN, "\tLTR329 ltr329_intensity_ch0:%d ltr329_intensity_ch1:%d\n", dp.ltr329_intensity_ch0, dp.ltr329_intensity_ch1);
	written = 0;
	to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	 
	chsnprintf(log_buf, MAX_LOG_LEN, "\tSTM32 temp:%d adc_vbat:%d\n", dp.stm32_temp, dp.adc_vbat);
	written = 0;
	to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	 
	chsnprintf(log_buf, MAX_LOG_LEN, "\tTMP100 temp_0:%d temp_1:%d\n", dp.tmp100_0_temp, dp.tmp100_1_temp);
	written = 0;
	to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	 
	chsnprintf(log_buf, MAX_LOG_LEN, "\tMPU9250 x:%d y:%d z:%d\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
	written = 0;
	to_write = strlen(log_buf);
	f_write(&log_file, log_buf, to_write, &written);
	if(to_write < written) LOG_ERR_LED();
	f_sync(&log_file);
#endif /* LOG_SD */
 
#if LOG_SERIAL
	chprintf((BaseSequentialStream*) &SD1, "%d DATA:\r\n", time);
	chprintf((BaseSequentialStream*) &SD1, "\tBME280 pressure:%d humidity:%d temperature:%d\r\n", dp.sen_i1_press, dp.sen_i1_hum, dp.sen_i1_temp);
	chprintf((BaseSequentialStream*) &SD1, "\tLTR329 ltr329_intensity_ch0:%d ltr329_intensity_ch1:%d\r\n", dp.ltr329_intensity_ch0, dp.ltr329_intensity_ch1);
	chprintf((BaseSequentialStream*) &SD1, "\tSTM32 temp:%d adc_vbat:%d\r\n", dp.stm32_temp, dp.adc_vbat);
	chprintf((BaseSequentialStream*) &SD1, "\tTMP100 temp_0:%d temp_1:%d\r\n", dp.tmp100_0_temp, dp.tmp100_1_temp);
	chprintf((BaseSequentialStream*) &SD1, "\tMPU9250 x:%d y:%d z:%d\r\n", dp.mpu9250_x_accel, dp.mpu9250_y_accel, dp.mpu9250_z_accel);
#endif/* LOG_SERIAL */
	
	return 0;
}

void log_close(void) {
	f_close(&log_file);
}

uint8_t log_image(void) {
	uint32_t time_s = LOG_TIME()/1000;
	char image_filename[MAX_FILENAME];
	chsnprintf(image_filename, MAX_FILENAME, "%s/img%d.jpg", log_dirname, time_s);
	uint32_t err = OV5640_Snapshot2SD(image_filename);
	if(err == FR_EXIST) {
		int img_num = 0;
		while(err == FR_EXIST && img_num++ < 1000) {
			chsnprintf(image_filename, MAX_FILENAME, "%s/img%d_%d.jpg", log_dirname, time_s, img_num);
			err = OV5640_Snapshot2SD(image_filename);
		}
	}
	return err;
}

void log_message(const char* msg, uint8_t level) {
	// TODO thread safe logging
	if(level == LOG_ERR) LOG_ERR_LED();	
	else if(level == LOG_WARN) LOG_WARN_LED();	
	if(level > LOG_LEVEL) return;
#if LOG_SD
	char time_message[MAX_LOG_LEN];
	chsnprintf(time_message, MAX_LOG_LEN, "%d %s:%s\n", LOG_TIME(), log_level_string[level], msg);
	unsigned int bytes_written;	
	f_write(&log_file, time_message, strlen(time_message), &bytes_written);
	f_sync();
#endif /* LOG_SD */ 
#if LOG_SERIAL
	chprintf((BaseSequentialStream*) &SD1, "%d %s:%s\r\n", LOG_TIME(), log_level_string[level], msg);
#endif /* LOG_SERIAL */
}
