#include "drv_mcp2515.h"


#define SPI_TIMEOUT         10
//#define MCP2515_CS_HIGH()   BSP_SET_BITS(SPI_CAN_CS_GPIO, SPI_CAN_CS_PIN)
//#define MCP2515_CS_LOW()    BSP_RESET_BITS(SPI_CAN_CS_GPIO, SPI_CAN_CS_PIN)


MCP2515_FUN mcp2515_fun;

/* 函数原型 */
static uint8_t SPI_Rx(void);
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);

/* MCP2515 切换到配置模式 */
uint8_t MCP2515_SetConfigMode(void)
{
    uint16_t loop = 10000;
  
    do 
    {
		/* CANCTRL Register Configuration 模式设置 */  
		MCP2515_WriteByte(MCP2515_CANCTRL, 0x80);
        /*  确认模式切换 */    
        if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x80)
        {
            return MCP2515_TRUE;
        }
      
        loop--;
    } while(loop > 0); 
  
    return MCP2515_FALSE;
}

/* MCP2515 切换到标准模式 */
uint8_t MCP2515_SetNormalMode(void)
{
    uint16_t loop = 30000;
    
    do {
            /* CANCTRL Register Normal 模式设置 */  
            MCP2515_WriteByte(MCP2515_CANCTRL, 0x00);		
        /* 确认模式切换 */    
        if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x00)
        return MCP2515_TRUE;
        
        loop--;
    } while(loop > 0);
    
    return MCP2515_FALSE;
}

/* MCP2515 切换到Sleep模式 */
uint8_t MCP2515_SetSleepMode(void)
{
    uint16_t loop = 30000;
    
    do {
            /* CANCTRL Register Sleep 模式设置 */  
            MCP2515_WriteByte(MCP2515_CANCTRL, 0x20);		
        /*  确认模式切换  */    
        if((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x20)
        return MCP2515_TRUE;
        
        loop--;
    } while(loop > 0);
    
    return MCP2515_FALSE;
}

/* MCP2515 SPI-Reset */
void MCP2515_Reset(void)
{
    mcp2515_fun.mcp_2515_cs_low();
        
    SPI_Tx(MCP2515_RESET);
        
    mcp2515_fun.mcp_2515_cs_high();
}

/* 读取1字节 */
uint8_t MCP2515_ReadByte (uint8_t address)
{
    uint8_t retVal;
    
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(MCP2515_READ);
    SPI_Tx(address);
    retVal = SPI_Rx();
        
    mcp2515_fun.mcp_2515_cs_high();
    
    return retVal;
}

/* Sequential Bytes 读取 */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length)
{
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(instruction);        
    SPI_RxBuffer(data, length);
        
    mcp2515_fun.mcp_2515_cs_high();
}

/* 写入1字节 */
void MCP2515_WriteByte(uint8_t address, uint8_t data)
{
    mcp2515_fun.mcp_2515_cs_low(); 
    
    SPI_Tx(MCP2515_WRITE);
    SPI_Tx(address);
    SPI_Tx(data);  
        
    mcp2515_fun.mcp_2515_cs_high();
}

/* Sequential Bytes 写入 */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data)
{  
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(MCP2515_WRITE);
    SPI_Tx(startAddress);
    SPI_TxBuffer(data, (endAddress - startAddress + 1));
    
    mcp2515_fun.mcp_2515_cs_high();
}

/* TxBuffer Sequential Bytes 写入 */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data)
{    
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(instruction);
    SPI_TxBuffer(idReg, 4);
    SPI_Tx(dlc);
    SPI_TxBuffer(data, dlc);
        
    mcp2515_fun.mcp_2515_cs_high();
}

/* TxBuffer 1 Bytes 写入 */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data)
{
    mcp2515_fun.mcp_2515_cs_low();;
    
    SPI_Tx(instruction);
    SPI_Tx(data);
            
    mcp2515_fun.mcp_2515_cs_high();
}

/* RTS 通过命令 TxBuffer 发送 */
void MCP2515_RequestToSend(uint8_t instruction)
{
	mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(instruction);
        
    mcp2515_fun.mcp_2515_cs_high();
}

/* MCP2515 Status 确认 */
uint8_t MCP2515_ReadStatus(void)
{
    uint8_t retVal;
    
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(MCP2515_READ_STATUS);
    retVal = SPI_Rx();
            
    mcp2515_fun.mcp_2515_cs_high();
    
    return retVal;
}

/* MCP2515 RxStatus 确认寄存器 */
uint8_t MCP2515_GetRxStatus(void)
{
    uint8_t retVal;
    
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(MCP2515_RX_STATUS);
    retVal = SPI_Rx();
            
    mcp2515_fun.mcp_2515_cs_high();
    
    return retVal;
}

/*  更改寄存器值  */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
    mcp2515_fun.mcp_2515_cs_low();
    
    SPI_Tx(MCP2515_BIT_MOD);
    SPI_Tx(address);
    SPI_Tx(mask);
    SPI_Tx(data);
            
    mcp2515_fun.mcp_2515_cs_high();
}

/*
*********************************************************************************************************
*	函 数 名: sf_SendByte
*	功能说明: 向器件发送一个字节，同时从MISO口线采样器件返回的数据
*	形    参:  _ucByte : 发送的字节值
*	返 回 值: 从MISO口线采样器件返回的数据
*********************************************************************************************************
*/
//static uint8_t sf_SendByte(uint8_t _ucValue)
//{
//	/* 等待上个数据未发送完毕 */
//	//while (SPI_I2S_GetFlagStatus(SPI_CAN, SPI_I2S_FLAG_TXE) == RESET);
//	
//	

//	/* 通过SPI硬件发送1个字节 */
//	//SPI_I2S_SendData(SPI_CAN, _ucValue);

//	/* 等待接收一个字节任务完成 */
//	//while (SPI_I2S_GetFlagStatus(SPI_CAN, SPI_I2S_FLAG_RXNE) == RESET);

//	/* 返回从SPI总线读到的数据 */
//	//return SPI_I2S_ReceiveData(SPI_CAN);
//}

/* SPI Tx Wrapper */
static void SPI_Tx(uint8_t data)
{
	//sf_SendByte(data);
	mcp2515_fun.mcp_2515_send_data(data);
	
}

/* SPI Tx Wrapper */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
	uint8_t i;
	for(i = 0;i<length;i++)
	{
		mcp2515_fun.mcp_2515_send_data(buffer[i]);
	}
}

/* SPI Rx Wrapper */
static uint8_t SPI_Rx(void)
{
    return mcp2515_fun.mcp_2515_send_data(0x00);
}

/* SPI Rx Wrapper */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
	uint8_t i;
	for(i = 0;i<length;i++)
	{
		buffer[i] = mcp2515_fun.mcp_2515_send_data(0x00);
	}
}
