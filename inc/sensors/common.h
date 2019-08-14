#ifndef __COMMON_H__
#define __COMMON_H__

enum SensorErr {
	SENSOR_OK		= 0x00,
	SENSOR_INV_DATA	= 0x00, /* Default data value, NOT an error code */
	SENSOR_COMM_ERR	= 0x01,
	SENSOR_INV_RESP	= 0x01 << 1,
	SENSOR_INV_CS	= 0x01 << 2,
};

#endif /* COMMON_H_ */

