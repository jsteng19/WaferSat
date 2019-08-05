#include "ch.h"
#include "hal.h"
#include "log.h"
#include "sd.h"
#include "gps.h"
#include "emmc.h"
#include "hal_fsmc_sdram.h"
#include "is42s16400j.h"
#include "ov5640.h"
#include "stdio.h"
#include "chprintf.h"
#include "membench.h"
#include "log.h"
#include "ff.h"
#include "pi2c.h"

int main(void) {
	int init_err = 0;
	halInit();
	chSysInit();
	log_init();
	init_err &= gps_init();
	pi2cInit();

	while (true) {
		char log[1024];
		char* ptr = log;
		ptr += snprintf(ptr, log + 1024 - ptr, "I2C Scan:");
		uint8_t addresses[127];
		uint8_t numd = I2C_scan(addresses);
		for(int i = 0; i < numd; i++) {
			ptr += snprintf(ptr, log + 1024 - ptr, " %02X", addresses[i]);
		}
		log_error(log);
		 
		LED_OK();
		chThdSleepMilliseconds(1000);
		LED_CLEAR();
		chThdSleepMilliseconds(1000);
	}
}
