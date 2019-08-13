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
#include "sensors/therm.h"
#include "common.h"
#include "pi2c.h"
#include "log.h"
//#include "ei2c.h"  //software i2c
#include <math.h>

/**
 * @brief	Initializes and configures the sensor 	
 * @note	Currently set to a 12 bit resolution and a maximum tolerance of 6 consecutive faults
 *
 * @return	The error state of the sensor
 * @see		SensorErr
 */
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

/**
 * @brief	Checks on the status of a given temperature sensor 
 * 
 * @param id	The id of the desired sensor. It can only be 0 or 1, which will refer
 *		to therm_0 and therm_1, respectively.
 * @return	The error state of the sensor
 * @see		SensorErr
 */
static enum SensorErr therm_status(uint8_t id)
{
	// TODO Enforce ID restriction

	uint8_t val;
	if(I2C_read8(THERM_ADDR | (id & 0x07), THERM_CONTR_REG, &val)) {
		return SENSOR_OK;
	} else {
	return SENSOR_COMM_ERR;
	}
}

/**
 * @brief	Gets temperature data from the two temperature sensors
 * @note	If a sensor is in an error state, the reading will be 0
 * 
 * @return	A structure containing the temperatures and an error state 
 * @see		therm_t
 */
therm_t therm_get(void)
{
	therm_t data = therm_t_init();
	
	if(therm_status(0)) {
		 uint16_t val;
		 if(!I2C_read16(THERM_ADDR | (0 & 0x07), THERM_TEMP_REG, &val)) {
			data.therm_0 =  SENSOR_INV_DATA;
			data.err |= SENSOR_COMM_ERR;
		} else {
			// convert .0625C/cnt to 0.01C/cnt
			data.therm_0 = (val >> 4) * (25 / 4);
		}
	}
	if(therm_status(1)) {
		 uint16_t val;
		 if(!I2C_read16(THERM_ADDR | (1 & 0x07), THERM_TEMP_REG, &val)) {
			data.therm_1 =  SENSOR_INV_DATA;
			data.err |= SENSOR_COMM_ERR;
		} else {
			// convert .0625C/cnt to 0.01C/cnt
			data.therm_1 = (val >> 4) * (25 / 4);
		}
	}
	
	return data;
}

/*
* TODO tmp_shutdown? (shutdown mode allows the temperature sensor to turn off
* all circuitry besides the serial interface. May be useful when we need to
* conserve power?)
*/
