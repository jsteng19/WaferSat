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
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dcmi.h"
#include "string.h"
#include <stdarg.h>
#include "chprintf.h"
#include "shell.h"
#include "chprintf.h"
#include "rt_test_root.h"
#include "dcmi_dma.h"
#include "hal_fsmc_sdram.h"
#include "membench.h"
#include "ff.h"
#include "pi2c.h"
#include "ov5640.h"
#include "is42s16400j.h"
#include "collector.h"
#include "debug.h"


#define IN_DRAM __attribute__((section(".ram7")))
uint8_t  OV5640_ram_buffer[OV5640_NUM_PIXELS*2] IN_DRAM;
volatile uint32_t OV5640_ram_buffer_length = 0;

int image_index = 0;

/*
 * FatFS object
 */

static FATFS SDC_FS;

/* FS mounted and ready.*/
static bool fs_ready = FALSE;

/* Generic large buffer.*/
static uint8_t fbuff[1024];


/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

/*===========================================================================*/
/* Camera related.                                                           */
/*===========================================================================*/

/*
 * Copy RGB565 data from OV5640_ram_buffer to file on SD card in .PPM P3 format
 */
static void write_rgb(void) {
	FIL fil;        /* File object */
	FRESULT fr;     /* FatFs return code */
	//  BYTE buffer[4096];   /* File copy buffer */
	UINT br, bw;         /* File read/write count */
	palSetPad(GPIOD, GPIOD_LED_BLUE);  //turn on red led on discovery
	/* Open a file */
	chprintf((BaseSequentialStream *)&SD_console, "Open file...");
	fr = f_open(&fil, "image1.ppm", FA_WRITE | FA_CREATE_ALWAYS);
	chprintf((BaseSequentialStream *)&SD_console, "done, return code = %d\r\n", fr);

	//header
	static char header [] = "P3\n0640 0480\n255\n";
	br = strlen(header);
	header[3] = OV5640_MAXX/1000%10 + '0';
	header[4] = OV5640_MAXX/100%10 + '0';
	header[5] = OV5640_MAXX/10%10 + '0';
	header[6] = OV5640_MAXX%10 + '0';

	header[8] = OV5640_MAXY/1000%10 + '0';
	header[9] = OV5640_MAXY/100%10 + '0';
	header[10] = OV5640_MAXY/10%10 + '0';
	header[11] = OV5640_MAXY%10 + '0';

	chprintf((BaseSequentialStream *)&SD_console, "Write header...");
	fr = f_write(&fil, header, br, &bw);
	if (fr || bw < br) { /* error or disk full */
		chprintf((BaseSequentialStream *)&SD_console, "write error!\r\n");
		return;
	} 
	else {
		chprintf((BaseSequentialStream *)&SD_console, "done, %d bytes written\r\n", bw);
	}

	//pixels

	unsigned y,x;
	uint16_t color;
	unsigned r,g,b;
	for(y = 0; y < OV5640_MAXY; y += 1) {
		for(x = 0; x < OV5640_MAXX; x += 1) {
			color = (OV5640_ram_buffer[2*(y*OV5640_MAXX + x)+1]>>8) + OV5640_ram_buffer[2*(y*OV5640_MAXX + x)];
			b = ((color&0x001F)<<3);    // 5bit blue
			g = ((color&0x07E0)>>3);    // 6bit green
			r = ((color&0xF800)>>8);    // 5bit red

			fbuff[0] = r/100%10 + '0';
			fbuff[1] = r/10%10 + '0';
			fbuff[2] = r%10 + '0';
			fbuff[3] = ' ';

			fbuff[4] = g/100%10 + '0';
			fbuff[5] = g/10%10 + '0';
			fbuff[6] = g%10 + '0';
			fbuff[7] = ' ';

			fbuff[8] = b/100%10 + '0';;
			fbuff[9] = b/10%10 + '0';
			fbuff[10] = b%10 + '0';
			fbuff[11] = ' ';

			fbuff[12] = '\n';
			br = 13;
			fr = f_write(&fil, fbuff, br, &bw);
			if (fr || bw < br){ /* error or disk full */
				chprintf((BaseSequentialStream *)&SD_console, "write error!\r\n");
				break;
			}
		}
		chprintf((BaseSequentialStream *)&SD_console, ".");
	}

	/* Close file */
	chprintf((BaseSequentialStream *)&SD_console, "Close file...");
	fr = f_close(&fil);
	chprintf((BaseSequentialStream *)&SD_console, "done, return code = %d\r\n", fr);
	palClearPad(GPIOD, GPIOD_LED_BLUE); //turn off red led on discovery

}

/*
 * Copy JPEG data from OV5640_ram_buffer to file on SD card
 */
static void write_jpeg(void) {
	FIL fil;        /* File object */
	FRESULT fr;     /* FatFs return code */
	//  BYTE buffer[4096];   /* File copy buffer */
	UINT br, bw;         /* File read/write count */
	uint32_t file_position;
	time_measurement_t mem_tmu;
	char filename[20];

	palSetPad(GPIOD, GPIOD_LED_BLUE);  //turn on red led on discovery

	chprintf((BaseSequentialStream *)&SD_console, "SDC  > File preview: ");
	if (OV5640_ram_buffer_length > 10) { //if file size is greater than 10 bytes, print abbreviated version
		for(file_position=0; file_position<5; file_position++) {
			chprintf((BaseSequentialStream *)&SD_console, "%02X, ", OV5640_ram_buffer[file_position]);
			//chprintf((BaseSequentialStream *)&SD_console, "%02X, %02X, ", (unsigned char)(OV5640_ram_buffer[file_position] & 0x00FF), (unsigned char)((OV5640_ram_buffer[file_position] & 0xFF00)>>8));
		}
		chprintf((BaseSequentialStream *)&SD_console, "%02X ... ", OV5640_ram_buffer[file_position]);
		for(file_position=OV5640_ram_buffer_length-5; file_position<OV5640_ram_buffer_length-1; file_position++) {
			chprintf((BaseSequentialStream *)&SD_console, "%02X, ", OV5640_ram_buffer[file_position]);
		}
		chprintf((BaseSequentialStream *)&SD_console, "%02X\r\n", OV5640_ram_buffer[file_position]);
	}
	else { //otherwise print all bytes
		for(file_position=0; file_position<OV5640_ram_buffer_length; file_position++) {
			chprintf((BaseSequentialStream *)&SD_console, "%02X, ", OV5640_ram_buffer[file_position]);
		}
		chprintf((BaseSequentialStream *)&SD_console, "%02X\r\n", OV5640_ram_buffer[file_position]);
	}
	/* Open a file */
	//if file exists, increment filename and try again

	chsnprintf(filename, sizeof(filename), "image%06d.jpg",image_index);
	chprintf((BaseSequentialStream *)&SD_console, "SDC  > Open %s ...", filename);
	fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
	if (fr == FR_EXIST) {
		chprintf((BaseSequentialStream *)&SD_console, "SDC  > file exists, search for first unused index...\r\n", filename);
		while(fr == FR_EXIST && image_index < 1000000) {
			image_index++;
			chsnprintf(filename, sizeof(filename), "image%06d.jpg",image_index);
			fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
		}
		chprintf((BaseSequentialStream *)&SD_console, "CAM  > Open %s ...", filename);
	}
	chprintf((BaseSequentialStream *)&SD_console, " done, return code = %d\r\n", fr);

	chprintf((BaseSequentialStream *)&SD_console, "SDC  > Write data...");
	chTMObjectInit(&mem_tmu);
	chTMStartMeasurementX(&mem_tmu);
	br = OV5640_ram_buffer_length;
	fr = f_write(&fil, OV5640_ram_buffer, br , &bw);
	chTMStopMeasurementX(&mem_tmu);
	if (fr || bw < br) { /* error or disk full */
		chprintf((BaseSequentialStream *)&SD_console, "write error!\r\n");
		return;
	} 
	else {
		chprintf((BaseSequentialStream *)&SD_console, "done, %.3f MB written, ", bw/1e6);
		chprintf((BaseSequentialStream *)&SD_console, "took, %d us, ", ((&mem_tmu)->last)/(STM32_SYSCLK/1000000));
		chprintf((BaseSequentialStream *)&SD_console, "%.2f MB/s\r\n", bw*1.0/ (((&mem_tmu)->last)/(STM32_SYSCLK/1000000)) );
		image_index++;
	}


	/* Close file */
	chprintf((BaseSequentialStream *)&SD_console, "SDC  > Close file...");
	fr = f_close(&fil);
	chprintf((BaseSequentialStream *)&SD_console, "done, return code = %d\r\n", fr);
	palClearPad(GPIOD, GPIOD_LED_BLUE); //turn off red led on discovery
}

/*===========================================================================*/
/* FSMC related.                                                             */
/*===========================================================================*/
/*
 * Erases the whole SDRAM bank.
 */
static void sdram_bulk_erase(void) {
	volatile uint8_t *p = (volatile uint8_t *)SDRAM_BANK_ADDR;
	volatile uint8_t *end = p + IS42S16400J_SIZE;
	while (p < end) *p++ = 0;
}
 
/*===========================================================================*/
/* FatFs related.                                                            */
/*===========================================================================*/


static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
	static FILINFO fno;
	FRESULT res;
	DIR dir;
	size_t i;
	char *fn;

	res = f_opendir(&dir, path);
	if (res == FR_OK) {
		i = strlen(path);
		while (((res = f_readdir(&dir, &fno)) == FR_OK) && fno.fname[0]) {
			if (FF_FS_RPATH && fno.fname[0] == '.')
			continue;
			fn = fno.fname;
			if (fno.fattrib & AM_DIR) {
				*(path + i) = '/';
				strcpy(path + i + 1, fn);
				res = scan_files(chp, path);
				*(path + i) = '\0';
				if (res != FR_OK)
				break;
			}
			else {
				chprintf(chp, "%s/%s\r\n", path, fn);
			}
		}
	}
	return res;
}


/*===========================================================================*/
/* SDC related                                                               */
/*===========================================================================*/

/*
 * Card insertion event.
 */
static void InsertHandler(eventid_t id) {
	FRESULT err;

	(void)id;
	/*
	* On insertion SDC initialization and FS mount.
	*/
#if HAL_USE_SDC
	if (sdcConnect(&SDCD1))
#else
	if (mmcConnect(&MMCD1))
#endif
		return;

	err = f_mount(&SDC_FS, "/", 1);
	if (err != FR_OK) {
#if HAL_USE_SDC
		sdcDisconnect(&SDCD1);
#else
		mmcDisconnect(&MMCD1);
#endif
		return;
	}
	fs_ready = TRUE;
}



/*
 * Card removal event.
 */
static void RemoveHandler(eventid_t id) {
	(void)id;
#if HAL_USE_SDC
	sdcDisconnect(&SDCD1);
#else
	mmcDisconnect(&MMCD1);
#endif
	fs_ready = FALSE;
}
/*===========================================================================*/
/* Card insertion monitor.                                                   */
/*===========================================================================*/

#define POLLING_INTERVAL                10
#define POLLING_DELAY                   10

/**
 * @brief   Card monitor timer.
 */
static virtual_timer_t tmr;

/**
 * @brief   Debounce counter.
 */
static unsigned cnt;

/**
 * @brief   Card event sources.
 */
static event_source_t inserted_event, removed_event;

/**
 * @brief   Insertion monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void tmrfunc(void *p) {
	BaseBlockDevice *bbdp = p;

	chSysLockFromISR();
	if (cnt > 0) {
		if (blkIsInserted(bbdp)) {
			if (--cnt == 0) {
				chEvtBroadcastI(&inserted_event);
			}
		}
		else
		cnt = POLLING_INTERVAL;
	}
	else {
		if (!blkIsInserted(bbdp)) {
			cnt = POLLING_INTERVAL;
			chEvtBroadcastI(&removed_event);
		}
	}
	chVTSetI(&tmr, TIME_MS2I(POLLING_DELAY), tmrfunc, bbdp);
	chSysUnlockFromISR();
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
static void tmr_init(void *p) {
	chEvtObjectInit(&inserted_event);
	chEvtObjectInit(&removed_event);
	chSysLock();
	cnt = POLLING_INTERVAL;
	chVTSetI(&tmr, TIME_MS2I(POLLING_DELAY), tmrfunc, p);
	chSysUnlock();
}


/*===========================================================================*/
/* Camera related                                                            */
/*===========================================================================*/

static void cmd_image_rgb(BaseSequentialStream *chp, int argc, char *argv[]) {
	FRESULT fr;     /* FatFs return code */

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: image\r\n");
		return;
	}

	chprintf((BaseSequentialStream *)&SD_console, "Camera Init...\r\n");
	OV5640_init_RGB();
	chprintf((BaseSequentialStream *)&SD_console, "Starting RGB Capture...\r\n");
	OV5640_Snapshot2RAM_RGB();   // Sample data from DCMI through DMA to RAM
	chprintf((BaseSequentialStream *)&SD_console, "capture done\r\n");

	if (!fs_ready) {
		chprintf(chp, "File System not mounted\r\n");
		return;
	}
	/* Register work area to the default drive */
	chprintf((BaseSequentialStream *)&SD_console, "Mounting file system...");
	fr = f_mount(&SDC_FS, "0:", 0);
	chprintf((BaseSequentialStream *)&SD_console, "done, return code = %d\r\n", fr);

	/* Write image data */
	write_rgb();
}

static void cmd_image(BaseSequentialStream *chp, int argc, char *argv[]) {
	FRESULT fr;     /* FatFs return code */

	(void)argv;
	if (argc > 0) {
		//chprintf(chp, "Usage: image\r\n");
		return;
	}

	while(1){
		//chprintf((BaseSequentialStream *)&SD_console, "Starting JPEG Capture...\r\n");
		palSetPad(GPIOD, GPIOD_LED_RED);
		OV5640_Snapshot2RAM();   // Sample data from DCMI through DMA to RAM
		palClearPad(GPIOD, GPIOD_LED_RED);

		//chprintf((BaseSequentialStream *)&SD_console, "capture done\r\n");

		if (!fs_ready) {
			//chprintf(chp, "File System not mounted\r\n");
			return;
		}
		/* Register work area to the default drive */
		//chprintf((BaseSequentialStream *)&SD_console, "SDC  > Mounting file system...");
		fr = f_mount(&SDC_FS, "0:", 0);
		//chprintf((BaseSequentialStream *)&SD_console, "done, return code = %d\r\n", fr);

		/* Write image data */
		write_jpeg();
		palSetPad(GPIOD, GPIOD_LED_GREEN);
		chThdSleepMilliseconds(1000);
		palClearPad(GPIOD, GPIOD_LED_GREEN);

	}
}

/*
 * Application entry point.
 */
int main(void) {
	static const evhandler_t evhndl[] = {
		InsertHandler,
		RemoveHandler,
	};
	event_listener_t el0, el1, el2;

	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/
	halInit();
	chSysInit();

	/*
	* Initialise FSMC for SDRAM.
	*/
	fsmcSdramInit();
	fsmcSdramStart(&SDRAMD, &sdram_cfg);
	sdram_bulk_erase();


    /*
     * Activates the  SDC driver 1 using default configuration.
     */
    sdcStart(&SDCD1, NULL);
    tmr_init(&SDCD1); //Activates the card insertion monitor.


	/*
	* Normal main() thread activity, in this demo it just performs
	* a shell respawn upon its termination.
	*/

	chEvtRegister(&inserted_event, &el0, 0);
	chEvtRegister(&removed_event, &el1, 1);
	chEvtRegister(&shell_terminated, &el2, 2);

	  /*
	  * Shell manager initialization.
	  */
	shellInit();

	/*
	* To be put in own thread after successful test
	*/

	/*
	* Initialize Camera
	*/
	OV5640_init();

	chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, TIME_MS2I(500)));

	// Executes capture command
	while (true) {
		// TODO abstract out contents of cmd_image so that it does not depend on a shell's context
		// TODO possible infinite loop in cmd_image?
		cmd_image(NULL, 0, NULL);
		chThdSleepMilliseconds(500);
	}
}
