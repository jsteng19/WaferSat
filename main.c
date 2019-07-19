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
#include "emmc.h"
#include "hal_fsmc_sdram.h"
#include "is42s16400j.h"
#include "ov5640.h"
#include "chprintf.h"
#include "membench.h"
#include "diskio.h"
#include "ff.h"
#include "log.h"

PARTITION VolToPart[] = {
	{0, 1},
	{1, 0}
};

int main(void) {
	int init_err = 0;
	halInit();
	chSysInit();
	emmc_init();
	init_err &= log_init();
	char message[100];
	 
	snprintf(message, 100, "Disk Status: %u", disk_status(1));
	log_message(message, LOG_INFO);
	snprintf(message, 100, "Disk Init: %u", disk_initialize(1));
	log_message(message, LOG_INFO);
	snprintf(message, 100, "Disk Status: %u", disk_status(1));
	log_message(message, LOG_INFO);
	 
	DWORD ptable[] = {100, 0, 0, 0};
	BYTE work[FF_MAX_SS];
	FRESULT f = f_fdisk(1, ptable, work);
	snprintf(message, 100, "Attempted to create fatfs; success %u", f);
	log_message(message, LOG_INFO);
	f = f_mkfs("1:", FM_ANY, 0, work, sizeof work);
	 
	snprintf(message, 100, "Attempted to create filesystem; success %u", f);
	log_message(message, LOG_INFO);
}
