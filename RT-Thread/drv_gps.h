#ifndef _DRV_GPS_H 
#define _DRV_GPS_H

#include "drv_config.h"
//#include "bsp_config.h"

#define GPS_RX_MAXLEN	  100// gps的串口接收最大字节数
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
	uint8_t posEff;				/* 位置数据有效 */
	char     NS;						/* 纬度半球N（北半球）或S（南半球） */
	char     EW;						/* 经度半球E（东经）或W（西经） 		*/
	double longd;						/* 经度 */
	double latd;						/* 纬度 */
	/*地面速度信息 */
	float f_trkDegT;			/* 以真北为参考基准的地面航向，精度0.1度 */
	float f_spdKts;				/* 地面速率精度0.01节*/
	float f_alt;					/* 海拔高度,精度0.01米*/
	float f_hdop;							/* 水平精度,精度0.01米 */
	uint8_t viewSatNum;			/* 可见卫星个数 */
	uint8_t usedSatNum;			/* 使用卫星个数 */
	/*UTC时间*/
	uint8_t Year;
	uint8_t  Month;
	uint8_t  Day;
	uint8_t  Hour;
	uint8_t  Min;
	uint8_t  Sec;
	//ANTENNA OPEN
	uint8_t antSta;					/* 1:正常 2:短路 3 :断开 , 但RTK模块没有这个功能 */
//	uint8_t seq;						/* 流水 用于判断定位模块是否在线 */
	
	uint8_t gpsQ;						/*定位质量*/
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
