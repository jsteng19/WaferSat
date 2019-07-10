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
#include "chprintf.h"
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
 
	// OV5640_init();
 
	while (true) {
		// int err = log_image();
		log_data();
		LOG_OK_LED();
		chThdSleepMilliseconds(1000);
		LOG_CLEAR_LED();
		chThdSleepMilliseconds(1000);
	}
}
