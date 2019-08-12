/*
* therm.c
* Library for TI THERM temperature sensor
*
* Created on: Nov 8, 2018
* 	Author: Peter Krogen
* Reformatted on 2019-08-12
* 	Author: Varun Iyer <varun_iyer@protonmail.ch>
 */

#include "ch.h"
#include "hal.h"
#include "therm.h"
#include "common.h"
#include "pi2c.h"
#include "log.h"
//#include "ei2c.h"  //software i2c
#include <math.h>

enum SensorErr therm_init(uint8_t id)
{
	if(I2C_write8(THERM_ADDR | (id & 0x07),
					THERM_CONTR_REG,
					THERM_CONSECUTIVE_FAULTS_6  | THERM_RESOLUTION_12_BIT )) {
		return SENSOR_COMM_ERR;
	} else {
		log_error("Unable to initialize therm sensor with ID %u!", id);
		return SENSOR_OK;
	}
}

enum SensorErr therm_status(uint8_t id)
{
	uint8_t val;
	if(I2C_read8(THERM_ADDR | (id & 0x07), THERM_CONTR_REG, &val)) {
		return SENSOR_OK;
	} else {
	return SENSOR_COMM_ERR;
	}
}

uint16_t therm_get(uint8_t id)
{
	uint16_t val;
	if(!I2C_read16(THERM_ADDR | (id & 0x07), THERM_TEMP_REG, &val)) {
		return SENSOR_INV_DATA;
	}
	return (val >> 4) * (25 / 4); // convert .0625C/count to 0.01C/count
}

/*
* TODO tmp_shutdown? (shutdown mode allows the temperature sensor to turn off
* all circuitry besides the serial interface. May be useful when we need to
* conserve power?)
*/
