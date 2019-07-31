#include "hal.h"
#include "hal_serial.h"
#include "gps.h"
#include "stdio.h"
#include "string.h"
#include "log.h"

static volatile gps_data_t gps_protected;
static mutex_t gps_mtx;

static THD_WORKING_AREA(gps_serial_wa, (MAX_LOG_LEN + GPS_MSG_SIZE) * 2);
static THD_FUNCTION(gps_serial_fn, args) {
	while(true) {
		// TODO use events instead of continuous poll
		char line[GPS_MSG_SIZE];
		if(gps_readline(line, GPS_MSG_SIZE) > 0) {
			gps_data_t data = gps_data_init();
			if(gps_parse(line, &data) == GPS_OK) gps_set(&data);
		}
	}
}
static thread_t* gps_serial_thd;
 
uint8_t gps_init(void) {
	/**
	Initializes GPIO and SerialDriver to communicate with GPS.
	Returns 0 on success and 1 on failure.
	**/
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	sdStart(&SD_GPS, &gps_conf);
	chMtxObjectInit(&gps_mtx);
	gps_data_t data = gps_data_init();
	gps_set(&data);
	gps_serial_thd = chThdCreateStatic(gps_serial_wa, sizeof(gps_serial_wa), NORMALPRIO, gps_serial_fn, NULL);
	return (SD_GPS.state == SD_READY) ? 0 : 1;	
}

gps_data_t gps_get() {
	/*
		Thread safe read of stored values.
		Returns stored struct.
	*/
	chMtxLock(&gps_mtx);
	chSysLock();
	gps_data_t data = gps_data_cpy(&gps_protected);
	chSysUnlock();
	chMtxUnlock(&gps_mtx);
	return data;
}
 
void gps_set(gps_data_t* data) {
	/*
		Thread safe write of stored values.
		Accepts pointer to gps data struct
		Returns void
	*/
	chMtxLock(&gps_mtx);
	chSysLock();
	gps_protected = gps_data_cpy(data);
	chSysUnlock();
	chMtxUnlock(&gps_mtx);
}

int gps_receive(uint8_t* buf, uint16_t buflen) {
	/**
	Receive a SerialDriver message from the GPS module.
	Return 0 upon success and a number according to the kind of failure.
	Error Codes:
		1 Failed to read start-of-sequence/no message available
		2 Msg too long/read buffer too small
		3 Incorrect checksum
		4 Failed to read end-of-sequence (message might still be okay)
	**/
	// Read start-of-sequence 
	uint8_t gpsbuf[100];
	uint8_t b = 0;
	uint8_t c = 0;
	uint8_t i = 0;
	while((	c = gps_getc()) &&
			i < 100 &&
			!(b == 0xA0 &&
			c == 0xA1)) {
		b = c;
		gpsbuf[i] = c;
		i++;
	}
	if(i == 100) {
		log_message("Failed to receive start of sequence!", LOG_ERR);
		return -1;
	}
	 
	char log_msg[MAX_LOG_LEN];
	char* ptr = log_msg;
	ptr += snprintf(log_msg, log_msg + MAX_LOG_LEN - ptr, "Skipped: ");
	for(int q = 0; q < i; q++) {
		ptr += snprintf(ptr, log_msg + MAX_LOG_LEN - ptr, "%02X ", gpsbuf[q]);
	}
	log_message(log_msg, LOG_CRITICAL);
		
	// Read msg-length
	uint8_t msg_len_buf[2];
	uint16_t read = gps_read(msg_len_buf, 2);
	uint16_t msg_len = (msg_len_buf[0] << 8) | msg_len_buf[1];

	snprintf(log_msg, MAX_LOG_LEN, "Length: %u", msg_len);
	log_message(log_msg, LOG_CRITICAL);
	 
	if(read != 2) {
		log_message("Did not read payload length from GPS.", LOG_ERR);
		return -1;
	}
	if(msg_len > buflen) {
		log_message("GPS read buffer is too small.", LOG_ERR);
		return -2;
	}

	//read mesage
	read = gps_read(buf, msg_len);
	int bytes_read = read;
	if(read != msg_len) {
		log_message("Failed to read complete message from GPS.", LOG_ERR);
		return -1;
	}

	// read/verify checksum
	uint8_t recv_cs = gps_getc();
	uint8_t calc_cs = 0;	
	for(int i = 0; i < msg_len; i++) calc_cs ^= buf[i];
	if(calc_cs != recv_cs) {
		log_message("Failed checksum verification of GPS message.", LOG_ERR);
	}
	
	// read end-of-sequence
	uint8_t endbuf[GPS_END_LEN];
	read = gps_read(endbuf, GPS_END_LEN);
	if(read != GPS_END_LEN || endbuf[0] != end_seq[0] || endbuf[1] != end_seq[1]) {
		snprintf(log_msg, MAX_LOG_LEN, "Read %02X %02X instead of 0x0D 0x0A.", endbuf[0], endbuf[1]);
		log_message(log_msg, LOG_ERR);
		return -1;
	}

	return bytes_read;
}

uint8_t gps_send(uint8_t* msg, uint16_t msg_len) {
	/**
	Transmit a message to the GPS according to the Skytraq binary protocol.
	The message **should include** the msg_id
	Return 0 on success, a number indicating position of failure upon failure
	**/
 
	// Transmit start-of-sequence to GPS
	int transmitted = gps_write(start_seq, GPS_START_LEN);
	if(transmitted != GPS_START_LEN) {
		log_message("Failed to write start-of-sequence to GPS.", LOG_ERR);
		return 1;
	}
 
	//transmit payload length
	//you need to add one to include the length of the message id
	uint8_t pl[] = {(uint8_t) (msg_len >> 8), (uint8_t) (msg_len & 0xFF)};
	transmitted = gps_write(pl, 2);
	if(transmitted != 2) {
		log_message("Failed to write payload length to GPS.", LOG_ERR);
		return 2;
	}

	//transmit message
	transmitted = gps_write(msg, msg_len);
	if(transmitted != msg_len) {
		log_message("Failed to write message to GPS.", LOG_ERR);
		return 4;
	}
 
	//transmit checksum
	uint8_t checksum = 0;
	for(int i = 0; i < msg_len; i++) checksum ^= msg[i];
	transmitted = gps_write(&checksum, 1);
	if(transmitted != 1) {
		log_message("Failed to write checksum to GPS.", LOG_ERR);
		return 5;
	}
 
	// Transmit end-of-sequence to GPS
	transmitted = gps_write(end_seq, GPS_END_LEN);
	if(transmitted != GPS_END_LEN) {
		log_message("Failed to write end-of-sequence to GPS.", LOG_ERR);
		return 6;
	}

	return 0;
}
 
uint16_t gps_readline(char* buf, uint16_t maxlen) {
	/**
		Reads one line of GPS output into the given buffer.
		Returns bytes read.
	**/
	char* ptr = buf;
	while(ptr < buf + maxlen && (*ptr = gps_getc()) && *ptr != '\n') if(*ptr != 0xFF) ptr++;
	*ptr = '\0';
	return ptr - buf;
}

gps_err_t gps_checksum(char* buf) {
	/**
		Calculates the GPS checksum
		returns GPS_OK if valid and GPS_INV if no delims ($ and *) are found and GPS_BAD_CS if checksum is wrong
	**/
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

gps_err_t gps_parse(char* buf, gps_data_t* data) {
	gps_err_t cs = gps_checksum(buf); 
	if(cs != GPS_OK) {
		return cs;
	}
	int found = gps_scan_gga(buf, data);
	if(found != 0) {
	//FIXME Please seee gps.h:21
		return GPS_INV;
	}
	return GPS_OK;
}
