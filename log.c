#include "sd.h"
#include "hal.h"
#include "string.h"
#include "chprintf.h"
#include "ff.h"
#include "collector.h"
#include "log.h"

FIL log_file;
char log_dirname[MAX_FILENAME]; 

uint8_t log_init(void) {
	/** 
	* Initializes the log file. 
	* @return 0 if success; a FATFS error otherwise
	**/
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
	return 0;	
}

uint8_t log_data(void) {
	dataPoint_t dp;
	getSensors(&dp);
	uint32_t time = LOG_TIME();
	char log_str[MAX_LOG_LEN]; 
	chsnprintf(log_str, MAX_LOG_LEN, "TEST\n");

	int len = 5;
	int written = 0;
	
	f_write(&log_file, "TEST\n", len, &written);
	f_sync(&log_file);
	return 0;
}

void log_close(void) {
	return f_close(&log_file);
}

uint8_t log_image(void) {
	uint32_t time_s = LOG_TIME()/1000;
	char image_filename[MAX_FILENAME];
	chsnprintf(image_filename, MAX_FILENAME, "%s/img%d.jpg", log_dirname, time_s);
	uint32_t err = OV5640_Snapshot2SD(image_filename);
	if(err == FA_EXIST) {
		int img_num = 0;
		while(err == FA_EXIST && img_num++ < 1000) {
			chsnprintf(image_filename, MAX_FILENAME, "%s/img%d_%d.jpg", log_dirname, time_s, img_num);
			err = OV5640_Snapshot2SD(image_filename);
		}
	}
	return err;
}

void log_message(const char* msg, uint8_t level) {
	if(level == LOG_ERR) LOG_ERR_LED();	
	else if(level == LOG_WARN) LOG_WARN_LED();	
	if(level > LOG_LEVEL) return;
	char time_message[MAX_LOG_LEN];
	chsnprintf(time_message, MAX_LOG_LEN, "%d %s:%s\n", LOG_TIME(), LOG_LEVEL_STRING[level], msg);
	uint8_t bytes_written;	
	f_write(&log_file, time_message, strlen(time_message), &bytes_written);
}
