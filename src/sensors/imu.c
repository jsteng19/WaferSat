/*
* IMU.c
* Library for IMU accelerometer
*
* 	Created on: Nov 8, 2018
* 		Author: Peter Krogen
* 	Reorganized on 2019-08-13
		Author: Varun Iyer <varun_iyer@protonmail.ch>
*/

#include "ch.h"
#include "hal.h"
#include "sensors/imu.h"
#include "pi2c.h"
#include "log.h"
#include <math.h>

enum SensorErr imu_init(void)
{
	uint8_t val;
	if(I2C_read8(IMU_ADDR, IMU_WHO_AM_I, &val)) {
		if(val != IMU_ACK_ID) {
			log_error("Failed to init IMU; read unexpected value %u!", val);
			return SENSOR_COMM_ERR | SENSOR_INV_RESP;
		}
		log_trace("Successfully intialized IMU.");
		return SENSOR_OK;
	} else {
		log_error("Failed to init IMU; did not read any response!");
		return SENSOR_COMM_ERR; // imu not found
	}
}

struct imu_t imu_get(void)
{
	uint16_t val;
	struct imu_t data = imu_t_init();
	if(!I2C_read16(IMU_ADDR, IMU_ACCEL_XOUT_H, &val)) {
		data.err |= SENSOR_COMM_ERR;
		data.x = SENSOR_INV_DATA;
	} else {
		data.x = (int16_t) val * 1000 / 16384;
	}
	if(!I2C_read16(IMU_ADDR, IMU_ACCEL_YOUT_H, &val)) {
		data.err |= SENSOR_COMM_ERR;
		data.y = SENSOR_INV_DATA;
	} else {
		data.y = (int16_t) val * 1000 / 16384;
	}
	if(!I2C_read16(IMU_ADDR, IMU_ACCEL_ZOUT_H, &val)) {
		data.err |= SENSOR_COMM_ERR;
		data.z = SENSOR_INV_DATA;
	} else {
		data.z = (int16_t) val * 1000 / 16384;
	}
	log_trace("Collected imu data " IMU_HUMAN_STR, IMU_T_FIELDS(&data));
	return data;
}
