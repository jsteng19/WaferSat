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
#include "emmc.h"
#include "hal_fsmc_sdram.h"
#include "ov5640.h"
#include "stdio.h"
#include "chprintf.h"
#include "log.h"
#include "ff.h"
#include "pi2c.h"
#include "sensors/common.h"

int main(void) {
	halInit();
	chSysInit();
	log_init();
	sensor_init();

	uint8_t addresses[127];
	uint8_t num = I2C_scan(addresses);
	for(uint8_t i = 0; i < num; i++) {
		log_info("Found I2C device at address 0x%02X", addresses[i]);
	}


	while (true) {
		log_data();
		LED_OK();
		chThdSleepMilliseconds(1000);
		LED_CLEAR();
		chThdSleepMilliseconds(1000);
	}
}
