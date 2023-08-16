#ifndef _DRV_CONFIG_H 
#define _DRV_CONFIG_H

#include "bsp_config.h"

#ifdef  FNE200_V1_2         /*FNE200 V1.2 硬件版本*/

#include "drv_led.h"        /*LED驱动*/
#include "drv_mcp2515.h"    /*SPI转CAN驱动*/

#endif

#ifdef  FNE200_V1_3         /*FNE200 V1.3 硬件版本*/

void driver_init(void);

#include "drv_mcp2515.h"     /*MCP2515驱动*/
#include "drv_gps.h"				 /*GPS驱动*/
void drv_mcp2515_init(void);
void drv_gps_init(void);

#endif

#ifdef  FNE100_1            /*FNE100-1 硬件版本*/

#include "drv_led.h"        /*LED驱动*/
#include "drv_mpu6050.h"    /*陀螺仪驱动*/

#endif

#ifdef  FNE100_2            /*FNE100-2 硬件版本*/

#include "drv_led.h"        /*LED驱动*/
#include "drv_gps.h"    /*GPS驱动*/

#endif

#endif
