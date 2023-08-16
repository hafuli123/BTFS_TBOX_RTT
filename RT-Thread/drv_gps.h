#ifndef _DRV_GPS_H 
#define _DRV_GPS_H

#include "drv_config.h"
//#include "bsp_config.h"

#define GPS_RX_MAXLEN	  100// gps�Ĵ��ڽ�������ֽ���
#define GPS_WORKMODE_INFO	0
#define GPS_WORKMODE_NMEA	1

typedef struct
{
	UART_HandleTypeDef gps_huart;
	struct rt_semaphore *gps_sem;
	
	void (*gps_init)(void);
	void (*gps_disinit)(void);
}GPS_VAR;

typedef struct
{
	uint8_t posEff;				/* λ��������Ч */
	char     NS;						/* γ�Ȱ���N�������򣩻�S���ϰ��� */
	char     EW;						/* ���Ȱ���E����������W�������� 		*/
	double longd;						/* ���� */
	double latd;						/* γ�� */
	/*�����ٶ���Ϣ */
	float f_trkDegT;			/* ���汱Ϊ�ο���׼�ĵ��溽�򣬾���0.1�� */
	float f_spdKts;				/* �������ʾ���0.01��*/
	float f_alt;					/* ���θ߶�,����0.01��*/
	float f_hdop;							/* ˮƽ����,����0.01�� */
	uint8_t viewSatNum;			/* �ɼ����Ǹ��� */
	uint8_t usedSatNum;			/* ʹ�����Ǹ��� */
	/*UTCʱ��*/
	uint8_t Year;
	uint8_t  Month;
	uint8_t  Day;
	uint8_t  Hour;
	uint8_t  Min;
	uint8_t  Sec;
	//ANTENNA OPEN
	uint8_t antSta;					/* 1:���� 2:��· 3 :�Ͽ� , ��RTKģ��û��������� */
//	uint8_t seq;						/* ��ˮ �����ж϶�λģ���Ƿ����� */
	
	uint8_t gpsQ;						/*��λ����*/
}GPS_INFO;

extern GPS_VAR gps_var;

void GPS_Open(void);
void GPS_Close(void);
uint16_t GPS_GetData(GPS_INFO* _info , uint8_t* _rbuf, uint8_t _fresh);
void GPS_Control(uint8_t _wmod);
static uint8_t GPS_ChkXor(uint8_t *_rbuf,uint16_t _len);
static uint8_t GPS_IdClas(uint8_t* _buf , uint16_t _len , GPS_INFO *_info);
static uint8_t GPS_GGA(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info);
static uint8_t GPS_RMC(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info);
static uint8_t GPS_GSV(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info);

#endif
