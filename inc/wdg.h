#ifndef __WDG_H__
#define __WDG_H__
#include "ch.h"
#include "hal.h"

/**
* set WDG deadline to a litte more than a second
*/
static const WDGConfig wdgcfg = {
	STM32_IWDG_PR_64,
	STM32_IWDG_RL(4000)
};
 
#define wdg_init() wdgStart(&WDGD1, &wdgcfg)
#define wdg_reset() do { \
		wdgReset(&WDGD1); \
		log_trace("Reset watchdog."); \
	} while(0)
 
#endif  /* __WDG_H__ */
