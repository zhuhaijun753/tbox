

#ifndef _GNSS_H
#define _GNSS_H

#include "data_type.h"

void task_gnss(void *data);
unsigned char read_gnss_state(void);
double read_gnss_longitude(void);
double read_gnss_latitude(void);
void read_gnss_utc_time(struct time_str *ptime);
void read_gnss_btc_time(struct time_str *ptime);
unsigned char read_satellite_num(void);
unsigned short int read_gnss_speed(void);
unsigned short int read_gnss_altitude(void);    					// ���� ��*/
unsigned short int read_gnss_heading(void);     					// ���� ��*/
unsigned char	read_bd_sate_num(void);					// ������������(����)
unsigned char read_gps_sate_num(void);					// ����΢������()
unsigned short int	read_gnss_hdop(void);
void close_gnss_modle(void);

#endif




