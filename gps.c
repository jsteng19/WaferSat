#include "hal.h"
#include "hal_serial.h"
#include "gps.h"
#include "log.h"

uint8_t gps_init() {
	/**
	Initializes GPIO and SerialDriver to communicate with GPS.
	Returns 0 on success and 1 on failure.
	**/
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	sdStart(&SD_GPS, &gps_conf);
	return (SD_GPS.state == SD_READY) ? 0 : 1;	
}

uint8_t gps_receive(uint8_t* buf, uint16_t buflen) {
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
	uint8_t startbuf[GPS_START_LEN];
	uint8_t read = sdReadTimeout(&SD_GPS, startbuf, GPS_START_LEN, GPS_TIMEOUT);
	if(read != GPS_START_LEN || startbuf[0] != start_seq[0] || startbuf[1] != start_seq[1]) {
		//FIXME TODO There is a possible error if a read fails partway through a message, 
		// all subsequent reads will fail because they will try to read a new message from the start of the corrupt buffer.
		log_message("Did not read start-of-sequence from GPS.", LOG_ERR);
		return 1;
	}

	// Read msg-length
	uint8_t msg_len_buf[2];
	read = sdReadTimeout(&SD_GPS, msg_len_buf, 2, GPS_TIMEOUT);
	uint16_t msg_len = (msg_len_buf[0] << 8) + msg_len_buf[1];	
	if(read != 2) {
		log_message("Did not read payload length from GPS.", LOG_ERR);
		return 1;
	}
	if(msg_len > buflen) {
		log_message("GPS read buffer is too small.", LOG_ERR);
		return 2;
	}

	//read mesage
	read = sdReadTimeout(&SD_GPS, buf, msg_len, GPS_TIMEOUT);
	if(read != msg_len) {
		log_message("Failed to read complete message from GPS.", LOG_ERR);
		return 1;
	}

	// read/verify checksum
	uint8_t recv_cs;
	read = sdReadTimeout(&SD_GPS, &recv_cs, 1, GPS_TIMEOUT);
	if(read != 1) {
		log_message("Failed to read checksum from GPS.", LOG_ERR);
		return 1;
	}
	uint8_t calc_cs = 0;	
	for(int i = 0; i < msg_len; i++) calc_cs ^= buf[i];
	if(calc_cs != recv_cs) {
		log_message("Failed checksum verification of GPS message.", LOG_ERR);
		return 3;
	}
	
	// read end-of-sequence
	uint8_t endbuf[GPS_END_LEN];
	read = sdReadTimeout(&SD_GPS, endbuf, GPS_END_LEN, GPS_TIMEOUT);
	if(read != GPS_END_LEN || endbuf[0] != end_seq[0] || endbuf[1] != end_seq[1]) {
		log_message("Did not read end-of-sequence from GPS.", LOG_ERR);
		return 1;
	}

	return 0;
}

uint8_t gps_send(uint8_t* msg, uint16_t msg_len) {
	/**
	Transmit a message to the GPS according to the Skytraq binary protocol.
	The message **should include** the msg_id
	Return 0 on success, a number indicating position of failure upon failure
	**/
 
	// Transmit start-of-sequence to GPS
	int transmitted = sdWriteTimeout(&SD_GPS, start_seq, GPS_START_LEN, GPS_TIMEOUT);
	if(transmitted != GPS_START_LEN) {
		log_message("Failed to write start-of-sequence to GPS.", LOG_ERR);
		return 1;
	}
 
	//transmit payload length
	//you need to add one to include the length of the message id
	uint8_t pl[] = {(uint8_t) (msg_len >> 8), (uint8_t) (msg_len & 0xFF)};
	transmitted = sdWriteTimeout(&SD_GPS, pl, 2, GPS_TIMEOUT);
	if(transmitted != 2) {
		log_message("Failed to write payload length to GPS.", LOG_ERR);
		return 2;
	}

	//transmit message
	transmitted = sdWriteTimeout(&SD_GPS, msg, msg_len, GPS_TIMEOUT);
	if(transmitted != msg_len) {
		log_message("Failed to write message to GPS.", LOG_ERR);
		return 4;
	}
 
	//transmit checksum
	uint8_t checksum = 0;
	for(int i = 0; i < msg_len; i++) checksum ^= msg[i];
	transmitted = sdWriteTimeout(&SD_GPS, &checksum, 1, GPS_TIMEOUT);
	if(transmitted != 1) {
		log_message("Failed to write checksum to GPS.", LOG_ERR);
		return 5;
	}
 
	// Transmit end-of-sequence to GPS
	transmitted = sdWriteTimeout(&SD_GPS, end_seq, GPS_END_LEN, GPS_TIMEOUT);
	if(transmitted != GPS_END_LEN) {
		log_message("Failed to write end-of-sequence to GPS.", LOG_ERR);
		return 6;
	}

	return 0;
}
 
uint8_t gps_ping() {	
	/**
	Asks the GPS its software version in order to confirm communication.
	0 if succesful comm, 1 if otherwise.
	**/ 
	uint8_t buf[GPS_MSG_SIZE];
	// Unique ID and payload for ping message
	uint8_t ping_msg[2] = {0x02, 0x00};
	uint8_t ping_len = 2;
	uint8_t err = gps_send(ping_msg, ping_len);
	if(err) return err;

	// This should return an ACK
	err = gps_receive(buf, GPS_MSG_SIZE);
	if(err) return err;
	if(buf[0] != GPS_ACK_ID) {
		return 1;
	}
	// we don't *actually* care about the software version.
	gps_receive(buf, GPS_MSG_SIZE);
	return 0;
}
