#include "hal.h"
#include "hal_serial.h"
#include "sensors/gps.h"
#include "stdio.h"
#include "string.h"
#include "log.h"

static volatile gps_data_t gps_protected;
static mutex_t gps_mtx;

static THD_WORKING_AREA(gps_serial_wa, GPS_MSG_SIZE * 4);
static THD_FUNCTION(gps_serial_fn, args) {
	while(true) {
		// TODO use events instead of continuous poll
		char line[GPS_MSG_SIZE];
		gps_data_t data = gps_get();
		gps_err_t msgs = GPS_NONE;
		// exit once all expected messages are collected
		while(gps_readline(line, GPS_MSG_SIZE) > 0 && msgs != GPS_MSG_MSK) {
			gps_err_t msg_type = gps_parse(line, &data);
			// If there’s an error, keep going
			if(msg_type & GPS_ERR_MSK) continue;
			if(msg_type & msgs) {
				// this is a message we have already collected
				// so we must have had an error in the last loop
				// so, reset the data
				data = gps_get();
				gps_parse(line, &data);
				continue;
			}
			// No errors and its a new message, so track of it
			msgs |= msg_type;	
		}
		gps_set(&data);
	}
}
static thread_t* gps_serial_thd;
 
/**
 * @brief	    Initializes GPIO and SerialDriver to communicate with GPS.
 * 
 * @return	    0 on success and 1 on failure.
 */
uint8_t gps_init(void) {
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	sdStart(&SD_GPS, &gps_conf);
	chMtxObjectInit(&gps_mtx);
	gps_data_t data = gps_data_init();
	gps_set(&data);
	gps_serial_thd = chThdCreateStatic(gps_serial_wa, sizeof(gps_serial_wa), NORMALPRIO, gps_serial_fn, NULL);
	return (SD_GPS.state == SD_READY) ? 0 : 1;
}

/**
 * @brief	    Thread safe read of stored values.
 * @return	    Stored struct.
 */
gps_data_t gps_get() {
	chMtxLock(&gps_mtx);
	chSysLock();
	gps_data_t data = gps_data_cpy(&gps_protected);
	chSysUnlock();
	chMtxUnlock(&gps_mtx);
	return data;
}
 
/**
 * @brief	    Thread safe write of stored values.
 * @param data	    Pointer to gps data struct
 * @see		    gps_data_t
 */
void gps_set(gps_data_t* data) {
	chMtxLock(&gps_mtx);
	chSysLock();
	gps_protected = gps_data_cpy(data);
	chSysUnlock();
	chMtxUnlock(&gps_mtx);
}
 
/**
 * @brief	    Reads one line of GPS output into the given buffer
 *
 * @param buf	    Buffer being read into
 * @param maxlen    Maximum length of message
 * @return	    Number of bytes read
 */
uint16_t gps_readline(char* buf, uint16_t maxlen) {
	char* ptr = buf;
	while(ptr < buf + maxlen && (*ptr = gps_getc()) && *ptr != '\n') if(*ptr != 0xFF) ptr++;
	*ptr = '\0';
	return ptr - buf;
}

/** 
 * @brief	    Calculates the GPS checksum
 * 
 * @param buf	    Buffer containing input to checksum
 * @return	    GPS_OK if valid and GPS_INV if no delims ($ and *) are found and GPS_BAD_CS if checksum is wrong
 */
gps_err_t gps_checksum(char* buf) {
	char* start = strchr(buf, (int)'$');
	char* end = strchr(buf, (int)'*');
	if(start == NULL || end == NULL) return GPS_INV;
	// We want to be just inside the $ and *
	char calc_cs = 0;
	char* ptr = start;
	// XOR every message character
	while(++ptr < end) calc_cs ^= *ptr;
	// Scan checksum from string
	unsigned int recv_cs;
	int matches = sscanf(end, "*%x", &recv_cs);
	if(matches != 1) {
		return GPS_INV;
	}

	if((uint8_t)recv_cs != (uint8_t) calc_cs) return GPS_BAD_CS;
	return GPS_OK;
}

/**
 * @brief	    Parse an NMEA line into GPS data.
 * 
 * @param buf	    NMEA message
 * @param data	    GPS Data struct to be filled
 * @return	    Error code and type of message parsed; if there is a parsing error, leave the GPS data untouched
 */
gps_err_t gps_parse(char* buf, gps_data_t* data) {
	gps_err_t cs = gps_checksum(buf); 
	if(cs != GPS_OK) {
		return cs;
	}
	gps_data_t test = gps_data_init();
	int found = gps_scan_gga(buf, &test);
	if(found == 9) {
		gps_scan_gga(buf, data);
		return GPS_OK | GPS_GGA;
	}
	found = gps_scan_zda(buf, &test);
	if(found == 4) {
		gps_scan_zda(buf, data);
		return GPS_OK | GPS_ZDA;
	}
	return GPS_INV;
}
