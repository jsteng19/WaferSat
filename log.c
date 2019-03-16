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
	//getSensors(&dp);
	uint32_t time = LOG_TIME();
	char log_str[MAX_LOG_LEN]; 
	chsnprintf(log_str, MAX_LOG_LEN, "TEST\n");

	int len = 5;
	int written = 0;
	
	f_write(&log_file, "TEST\n", len, &written);
	f_sync(&log_file);
	return 0;
}
