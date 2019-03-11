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
#include "string.h"
#include "hal_fsmc_sdram.h"
#include "ff.h"

static FATFS SDC_FS;

uint8_t sd_init(void) {
	/*
	* On insertion SDC initialization and FS mount.
	*/
#if HAL_USE_SDC
	if (sdcConnect(&SDCD1))
#else
	if (mmcConnect(&MMCD1))
#endif
		return 1;

	int err = f_mount(&SDC_FS, "/", 1);
	if (err != FR_OK) {
#if HAL_USE_SDC
		sdcDisconnect(&SDCD1);
#else
		mmcDisconnect(&MMCD1);
#endif
		return 1;
	}
	return 0;
}

uint8_t sd_write(FIL* fil, char* string) {
	unsigned int len = strlen(string);
	unsigned int written;
	int failure = f_write(fil, string, len, &written);
	if(failure) {
		//TODO handle error
		return 1;
	}
	if(len < written) {
		//TODO handle error
		return 2;
	}
	return 0;
}
 
