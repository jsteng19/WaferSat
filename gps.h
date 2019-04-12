#ifndef __GPS_H__
#define __GPS_H__
 
#include "hal_serial.h"

#define SD_GPS SD3
#define GPS_TIMEOUT TIME_MS2I(50)
#define GPS_MSG_SIZE 1024

const SerialConfig gps_conf = {9600, 0, 0, 0};
 
const uint8_t start_seq[] = {0xA0, 0xA1};
#define GPS_START_LEN 2
const uint8_t end_seq[] = {0x0D, 0x0A};
#define GPS_END_LEN 2
const uint8_t ping_msg[] = {0x02, 0x00};
#define GPS_PING_LEN 2
#define GPS_ACK_ID 0x83

uint8_t gps_init(uint8_t* buf, uint16_t buflen);
uint8_t gps_send(uint8_t* msg, uint16_t msg_len);
uint8_t gps_ping();
