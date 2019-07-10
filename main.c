/*
    Copyright (C) 2013-2015 Andrea Zoppi

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "log.h"
#include "sd.h"
#include "hal_fsmc_sdram.h"
#include "is42s16400j.h"
#include "ov5640.h"
#include "stdio.h"
#include "chprintf.h"
#include "membench.h"
#include "ff.h"

int main(void) {
	int init_err = 0;
	halInit();
	chSysInit();
	init_err &= sd_init();
	init_err &= log_init();

	pi2cInit();

	//Initialize SDRAM
	fsmcSdramInit();
	fsmcSdramStart(&SDRAMD, &sdram_cfg);
 
 	//FIXME if camera is not attached camera init hangs
	//OV5640_init();
	 
	static uint8_t int_buf[64*1024];
	static membench_t membench_ext = {
		SDRAM_START,
		SDRAM_SIZE,
	};

	static membench_t membench_int = {
		int_buf,
		sizeof(int_buf),
	};

	static membench_result_t membench_result_ext2int;
	static membench_result_t membench_result_int2ext;
 
	while (true) {
	 
		log_data();
		membench_run(&membench_ext, &membench_int, &membench_result_int2ext);
		membench_run(&membench_int, &membench_ext, &membench_result_ext2int);
		char log_msg[MAX_LOG_LEN];
		log_message("OK\nRAM Performance (MB/s) memset, memcpy, memcpy_dma, memcmp\n", LOG_VERBOSE);
		snprintf(log_msg, MAX_LOG_LEN, "int2ext:                 %lu %lu %10lu %lu \r\n",
		membench_result_int2ext.memset/1000000,
		membench_result_int2ext.memcpy/1000000,
		membench_result_int2ext.memcpy_dma/1000000,
		membench_result_int2ext.memcmp/1000000);
		log_message(log_msg, LOG_VERBOSE);

		snprintf(log_msg, MAX_LOG_LEN, "ext2int:                 %lu %lu %10lu %lu \r\n",
		membench_result_ext2int.memset/1000000,
		membench_result_ext2int.memcpy/1000000,
		membench_result_ext2int.memcpy_dma/1000000,
		membench_result_ext2int.memcmp/1000000);
		log_message(log_msg, LOG_VERBOSE);

		LOG_OK_LED();
		chThdSleepMilliseconds(1000);
		LOG_CLEAR_LED();
		chThdSleepMilliseconds(1000);
	}
}
