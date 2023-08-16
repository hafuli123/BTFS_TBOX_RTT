#include "drv_config.h"

void spi_set_cs_high(void);
void spi_set_cs_low(void);
uint8_t spi_can_send_data(uint8_t ucValue);

void usart_gps_init(void);
void usart_gps_disinit(void);
extern struct rt_semaphore gps_sem;

void driver_init(void)
{
#ifdef  FNE200_V1_2         /*FNE200 V1.2 硬件版本*/
	drv_led();

#endif

#ifdef  FNE200_V1_3         /*FNE200 V1.3 硬件版本*/
	drv_mcp2515_init();
	drv_gps_init(); 

#endif

#ifdef   FNE100_1            /*FNE100-1 硬件版本*/
	drv_adc();


#endif

#ifdef   FNE100_2            /*FNE100-2 硬件版本*/
	drv_beep();

#endif
}

/* MCP2515 驱动初始化 */

void drv_mcp2515_init(void)
{
	mcp2515_fun.mcp_2515_cs_high = spi_set_cs_high; 
	mcp2515_fun.mcp_2515_cs_low = spi_set_cs_low;
	mcp2515_fun.mcp_2515_send_data = spi_can_send_data;
	
}

void spi_set_cs_high(void)
{
	HAL_GPIO_WritePin(CAN_SPI_NSS_GPIO, CAN_SPI_NSS_PIN, GPIO_PIN_SET);
  //GPIO_SetBits(SPI_CAN_CS_GPIO,SPI_CAN_CS_PIN);
}

void spi_set_cs_low(void)
{
	HAL_GPIO_WritePin(CAN_SPI_NSS_GPIO, CAN_SPI_NSS_PIN, GPIO_PIN_RESET);
}

uint8_t spi_can_send_data(uint8_t ucValue)
{
	uint8_t ucRcValue;
	
	HAL_SPI_TransmitReceive(&CAN3_SPI, &ucValue, &ucRcValue, 1,100);
	
	return ucRcValue;
} 

/* GPS 驱动初始化 */
void drv_gps_init(void) 
{		
  gps_var.gps_huart = GPS_USART;   
	gps_var.gps_sem = &gps_sem;     
	gps_var.gps_init = usart_gps_init;
	gps_var.gps_disinit = usart_gps_disinit;
}

void usart_gps_init(void)
{
	/* GPS串口开启 */
	MX_USARTx_UART_Init(&GPS_USART); 
	/* 开启GPS串口接收中断 */
  HAL_NVIC_SetPriority(GPS_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(GPS_IRQn);
	__HAL_UART_ENABLE_IT(&GPS_USART , UART_IT_RXNE);
}

void usart_gps_disinit(void)
{
	/* 关闭GPS串口 */
	HAL_UART_MspDeInit(&GPS_USART);
	/* 关闭GPS串口接收中断 */
	HAL_NVIC_DisableIRQ(GPS_IRQn);
}

