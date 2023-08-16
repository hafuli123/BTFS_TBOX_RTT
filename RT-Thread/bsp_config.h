#ifndef _BSP_CONFIG_H 
#define _BSP_CONFIG_H

#include <stdint.h>
#include "stm32f4xx.h"
#include "can.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "sdio.h"
#include "usb_otg.h"

#include "string.h"
#include "stdio.h"
#include <rtthread.h>

//#define FNE200_V1_2         /*FNE200 V1.2 硬件版本*/
#define FNE200_V1_3     		  /*FNE200 V1.3 硬件版本*/
//#define FNE100_1          	/*FNE100-1 硬件版本*/
//#define FNE100_2          	/*FNE100-2 硬件版本*/



#ifdef  FNE200_V1_2         /*FNE200 V1.2 硬件版本*/

 /* GPS指示灯引脚定义 */
#define led_gps_pin         GPIO_Pin_1     
#define led_gps_gpio        GPIOA

 /* SPI转CAN引脚定义 */
#define SPI_CAN             SPI1
#define SPI_CAN_CS_PIN		GPIO_Pin_4
#define SPI_CAN_CS_GPIO		GPIOA
#endif

#ifdef  FNE200_V1_3         /*FNE200 V1.3 硬件版本*/
/*4G芯片远程唤醒信号读取引脚定义*/
#define RI_IN_PIN					GPIO_PIN_0
#define RI_IN_GPIO        			GPIOA

/*4G芯片远程唤醒开关引脚定义*/
#define GPRS_WAKE_PW_PIN 			GPIO_PIN_1
#define GPRS_WAKE_PW_GPIO 			GPIOA

/*串口2发送引脚定义*/
#define USART2_TX_PIN 				GPIO_Pin_2
#define USART2_TX_GPIO 				GPIOA

/*串口2接收引脚定义*/
#define USART2_RX_PIN 				GPIO_Pin_3
#define USART2_RX_GPIO 				GPIOA

/**/

/*SPI1转CAN3片选信号引脚定义*/
#define SPI1_CAN_NSS_PIN  			GPIO_PIN_4
#define SPI1_CAN_NSS_GPIO 			GPIOA

/*SPI1转CAN3串行时钟引脚定义*/
#define SPI1_CAN_SCK_PIN  			GPIO_PIN_5
#define SPI1_CAN_SCK_GPIO 			GPIOA

/*SPI1转CAN3主收从发信号引脚定义*/
#define SPI1_CAN_MISO_PIN  			GPIO_PIN_6
#define SPI1_CAN_MISO_GPIO 			GPIOA

/*SPI1转CAN3主发从收信号引脚定义*/
#define SPI1_CAN_MOSI_PIN  			GPIO_PIN_7
#define SPI1_CAN_MOSI_GPIO 			GPIOA

/*电池允许充电开关引脚定义*/
#define BAT_PW_PIN				 	GPIO_PIN_8
#define BAT_PW_GPIO 				GPIOA

/*串口1发送引脚定义*/
#define USART1_TX_PIN 				GPIO_Pin_9
#define USART1_TX_GPIO 				GPIOA

/*串口1接收引脚定义*/
#define USART1_RX_PIN 				GPIO_Pin_10
#define USART1_RX_GPIO 				GPIOA

/*USBDM引脚定义*/
#define USB_DM_PIN					GPIO_PIN_11
#define USB_DM_GPIO       			GPIOA

/*USBDP引脚定义*/
#define USB_DP_PIN					GPIO_PIN_12
#define USB_DP_GPIO       			GPIOA

/*SPI2片选引脚1*/
#define SPI2_NSS1_PIN      			GPIO_PIN_15
#define SPI2_NSS1_GPIO				GPIOA

/*CAN休眠引脚定义*/
#define CAN_STB_PIN					GPIO_PIN_0
#define CAM_STB_GPIO				GPIOB

/*5.3V/5.0V电源变压器使能*/
#define PW_VCC_PIN       			GPIO_PIN_1
#define PW_VCC_GPIO  				GPIOB

/*蜂鸣器*/
#define BEEP_PIN					GPIO_PIN_3
#define BEEP_GPIO					GPIOB

/*ACC唤醒信号*/
#define ACC_IN_PIN					GPIO_PIN_4
#define ACC_IN_GPIO 				GPIOB

/*CAN2接收引脚定义*/
#define CAN2_RX_PIN					GPIO_PIN_5
#define CAN2_RX_GPIO				GPIOB

/*CAN2发送引脚定义*/
#define CAN2_TX_PIN					GPIO_PIN_6
#define CAN2_TX_GPIO				GPIOB

/*电源自锁引脚定义*/
#define CPU_INT_PIN 				GPIO_PIN_7
#define CPU_INT_GPIO				GPIOB	

/*CAN1接收引脚定义*/
#define CAN1_RX_PIN					GPIO_PIN_8
#define CAN1_RX_GPIO				GPIOB

/*CAN2发送引脚定义*/
#define CAN1_TX_PIN					GPIO_PIN_9
#define CAN1_TX_GPIO				GPIOB

/*USART3发送引脚定义*/
#define USART3_TX_PIN				GPIO_PIN_10
#define USART3_TX_GPIO				GPIOB

/*USART3接收引脚定义*/
#define USART3_RX_PIN				GPIO_PIN_11
#define USART3_RX_GPIO				GPIOB 

/*SPI2片选引脚2*/
#define SPI2_NSS2_PIN				GPIO_PIN_12
#define SPI2_NSS2_GPIO 				GPIOB

/*SPI2串行时钟引脚定义*/
#define SPI2_SCK_PIN				GPIO_PIN_13
#define SPI2_SCK_GPIO				GPIOB

/*SPI2主收从发引脚定义*/
#define	SPI2_MISO_PIN				GPIO_PIN_14
#define SPI2_MISO_GPIO				GPIOB	

/*SPI2主发从收引脚定义*/
#define SPI2_MOSI_PIN				GPIO_PIN_15
#define	SPI2_MOSI_GPIO				GPIOB

/*电池ADC引脚定义*/
#define ADC_VBAT_PIN				GPIO_PIN_0
#define ADC_VBAT_GPIO				GPIOC

/*GPRS指示灯引脚定义*/
#define GPRS_LED_PIN				GPIO_PIN_1
#define GPRS_LED_GPIO     			GPIOC

/*GPS指示灯引脚定义*/
#define GPS_LED_PIN					GPIO_PIN_2
#define GPS_LED_GPIO     			GPIOC

/*CAN指示灯引脚定义*/
#define CAN_LED_PIN					GPIO_PIN_3
#define CAN_LED_GPIO     			GPIOC

/*CAN3_SPI片选引脚定义*/
#define CAN_SPI_NSS_PIN				GPIO_PIN_4
#define CAN_SPI_NSS_GPIO			GPIOC

/*电源ADC引脚定义*/
#define ADC_VCC_PIN					GPIO_PIN_5
#define ADC_VCC_GPIO				GPIOC

/*端子输出信号引脚定义*/
#define MCU_IO_PIN					GPIO_PIN_6
#define MCU_IO_GPIO					GPIOC

/*3.3V供电使能引脚定义*/
#define V3_3_PW_PIN					GPIO_PIN_7
#define V3_3_PW_GPIO				GPIOC

/*SDIO_D0引脚定义*/
#define SDIO_D0_PIN					GPIO_PIN_8
#define SDIO_D0_GPIO				GPIOC

/*SDIO_D1引脚定义*/
#define SDIO_D1_PIN					GPIO_PIN_9
#define SDIO_D1_GPIO				GPIOC

/*SDIO_D2引脚定义*/
#define SDIO_D2_PIN					GPIO_PIN_10
#define SDIO_D2_GPIO				GPIOC

/*SDIO_D3引脚定义*/
#define SDIO_D03_PIN				GPIO_PIN_11
#define SDIO_D3_GPIO				GPIOC

/*SDIO_CK引脚定义*/
#define SDIO_CK_PIN					GPIO_PIN_12
#define SDIO_CK_GPIO				GPIOC

/*EC200唤醒引脚定义*/
#define GPRS_DTR_PIN				GPIO_PIN_13
#define	GPRS_DTR_GPIO				GPIOC

/*SDIO_CMD引脚定义*/
#define SDIO_CMD_PIN				GPIO_PIN_2
#define SDIO_CMD_GPIO				GPIOD

/*RCC_OSC_IN引脚定义*/
#define OSC_IN_PIN					GPIO_PIN_0
#define OSC_IN_GPIO					GPIOH

/*RCC_OSC_OUT引脚定义*/
#define OSC_OUT_PIN					GPIO_PIN_0
#define OSC_OUT_GPIO				GPIOH

/*上位机对应的USART*/
#define HOST_CPT_USART				huart1//UASRT1

/*GPRS对应的USART*/
#define  GPRS_USART 				huart2//UASRT2

/*GPS对应的USART*/
#define  GPS_USART 			 huart3//UASRT3 
#define	 GPS_BAUDRATE		 115200
#define	 USART3_BAUDRATE GPS_BAUDRATE
#define	 GPS_IRQHANDLER	 USART3_IRQHandler
#define  GPS_IRQn				 USART3_IRQn

#define  GPS_NMEA_V3					/* 该GPS芯片使用的NMEA版本 */		
//#define  GPS_NMEA_V4			 

/*CAN3对应的SPI*/
#define CAN3_SPI          			hspi1//SPI1

/*加密芯片对应SPI*/
#define ENCRYPT_SPI					hspi2//SPI2

/*ADC采集*/
#define VCC_ADC						ADC1

#endif

#ifdef  FNE100_1            /*FNE100-1 硬件版本*/

 /* GPS指示灯引脚定义 */
#define led_gps_pin         GPIO_Pin_3     
#define led_gps_gpio        GPIOA

#endif


#ifdef FNE100_2
/* GPS指示灯引脚定义 */
#define led_gps_pin         GPIO_Pin_4      
#define led_gps_gpio        GPIOA

#endif



#endif
