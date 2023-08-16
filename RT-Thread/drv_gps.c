#include "drv_gps.h" 

GPS_VAR gps_var;

uint8_t gpsRxBuf[GPS_RX_MAXLEN] , gpsSavBuf[GPS_RX_MAXLEN];
__IO uint8_t gpsRxStep=0;
__IO uint16_t rxCnt=0;		/* 已接收到的字节数 */
static __IO uint16_t gpsRxLen=0;
static __IO uint8_t sSum, ucSum; /* 校验NMEA语句时使用 */
static __IO uint32_t i32_ang, i32_min, i32_dec;
static __IO uint8_t vuNum[9];
static __IO uint8_t wMode;

/* 
	GPS的串口接收中断服务函数
*/
static uint8_t data;int bana;
//extern struct rt_semaphore gps_sem;
void GPS_IRQHANDLER(void)
{
	if((__HAL_UART_GET_FLAG(&gps_var.gps_huart,UART_FLAG_RXNE)!= RESET) ||(__HAL_UART_GET_FLAG(&gps_var.gps_huart, UART_FLAG_ORE) != RESET)){
		data=READ_REG(gps_var.gps_huart.Instance->DR); // 读取数据
		/* 处理溢出中断 */
		if(__HAL_UART_GET_FLAG(&gps_var.gps_huart, UART_FLAG_ORE) != RESET)
		{
			/* 清除溢出中断 */
			__HAL_UART_CLEAR_FLAG(&gps_var.gps_huart, UART_FLAG_ORE);
		}	
		
		/* 收到NMEA语句的开头帧$ */		
		if(!gpsRxStep){
			if(data == '$'){
				gpsRxBuf[0]=data;
				rxCnt=1;
				gpsRxLen=1;
				gpsRxStep=1;
			}	
		}
		else if(gpsRxStep==1){
			gpsRxBuf[rxCnt]=data;
			rxCnt++;
			gpsRxLen++;
			if(data=='\n'){
				rxCnt=0;
				/* 置为2，采集先暂停住，等待采集的重开 */
				/* 此时数组里一直留着这一条NMEA语句 */
				/* GPS_GetData()对这最近一条NMEA数据收集起来后，再在那里把gpsRxStep置为0，中断这里便可开启新的一条采集 */
				gpsRxStep=2;
				/* 发送信号量，负责解析的线程开启工作 */
				rt_sem_release(gps_var.gps_sem); 
				return;
			}
			if(rxCnt>=GPS_RX_MAXLEN-1){
				rxCnt=0;
				gpsRxLen=0;
				gpsRxStep=0;
			}			
		}
	}
	return;
}

/*
	GPS模块开启
*/
void GPS_Open(void){
	gps_var.gps_init();
}

/*
	GPS模块关闭
*/
void GPS_Close(void){
	gps_var.gps_disinit();
}

/*
	获取数据，有两种工作模式获取不同的两种数据：

	默认方式0：不断对采集到的NMEA语句进行解析，把结果放入GPS_INFO结构体中，Get GPS_INFO Data，获取GPS数据；
						 输入参数GPS_INFO结构体，把解析后的信息数据存入结构体中。
	方式1：不需解析，直接给出串口最近收到的一条NMEA语句，Get NMEA Data，获取收到的NMEA语句。
				 输入参数_rbuf是想存放这条NMEA语句的缓冲区，注意_rbuf的缓冲区大小需要大于100，_fresh为存放完语句进入_rbuf后是否刷新，即中断采集一条新的NMEA语句，0则刷新。

	返回值：
	模式0时，返回0表明当前解析的这句NMEA语句无效，没有所需信息；返回1、2、3分别表明刚刚解析的是那种NMEA语句，GGA、RMC或GSV
	模式1时，返回值1表明这条NMEA语句的长度
*/
uint16_t GPS_GetData(GPS_INFO* _info , uint8_t* _rbuf, uint8_t _fresh)
{
	uint16_t _len;
	_len=gpsRxLen;
	
	switch(wMode){
		case 0:{
			memcpy((uint8_t*)gpsSavBuf , (uint8_t*)gpsRxBuf , gpsRxLen);
			gpsRxStep=0;
			if(GPS_ChkXor( (uint8_t*)gpsSavBuf ,  _len)){
				return GPS_IdClas((uint8_t*)gpsSavBuf , _len , _info);
			}	
		}
		case 1:{
			memcpy((uint8_t*)_rbuf , (uint8_t*)gpsRxBuf , gpsRxLen);
			if(!_fresh){	//是否需要刷新，即开始重新采集新一条NMEA语句，0则进行刷新
				gpsRxStep=0;
			}
			return _len;
		}
	}
	return 0;
}

/*
	对GPS的数据采集进行选择，选择需要获得哪种数据
*/
void GPS_Control(uint8_t _wmod)
{
	wMode = _wmod;
}

/*
	对一条NMEA语句进行校验。
	返回值：校验失败返回0，校验正确返回1.
*/
static uint8_t GPS_ChkXor(uint8_t *_rbuf,uint16_t _len)
{
	sSum=0;
	ucSum=0;
	if(_len<3){
		return 0;
	}
	/* 如果没有校验字节，也认为出错 */
	if(_rbuf[_len-5]!='*'){
		return 0;
	}	
	
	for(int i = 1; i < _len - 5; i++){
		/* 不允许出现非ASCII字符 */
		if((_rbuf[i] & 0x80) || (_rbuf[i] == 0))
		{
			return 0;
		}		
		ucSum = ucSum ^ _rbuf[i];		
	}
	sscanf((char*)&_rbuf[_len - 4],"%02hhx",&sSum);  
	if(sSum == ucSum){
		return 1;
	}
	return 0;
}

/*
	对NMEA语句的ID进行分类，不同的种类执行不同的解析函数
	返回值
*/
static uint8_t GPS_IdClas(uint8_t* _buf , uint16_t _len , GPS_INFO *_info) 
{
	if( (_buf[3]=='G')&&(_buf[4]=='G')&&(_buf[5]=='A') ){
		return GPS_GGA(_buf , _len , _info);
	}
	else if( (_buf[3]=='R')&&(_buf[4]=='M')&&(_buf[5]=='C') ){
		return GPS_RMC(_buf , _len , _info);
	}
	else if((_buf[3]=='G')&&(_buf[4]=='S')&&(_buf[5]=='V')){
		return GPS_GSV(_buf , _len , _info);
	}
	return 0;
}

/*
	对GGA语句进行解析的函数
*/
static uint8_t GPS_GGA(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA语句的一块中是否拥有数据。有数据时NMEA为两个逗号中间有数据例如‘,xxx,’，当没有数据的时候这块为‘,,’。 
	p_src = (uint8_t*)&_abuf[7];	
	
	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//这一块里有数据‘,xxx,’
		}	
		if(avail){
			avail=0;
			switch(i){
				case 0:{
					//get time
					sscanf((const char *)destBuf,"%2hhu%2hhu%2hhu",&_info->Hour,&_info->Min,&_info->Sec);
					_info->Hour = (_info->Hour+8)%24;					
					break;
				}
				case 1:{
					//get latitude
					sscanf((const char *)destBuf,"%2u%2u.%6u",&i32_ang,&i32_min,&i32_dec);
					_info->latd = (double)i32_ang+ ((double)i32_min + (double)i32_dec/1000000.000000f)/60.000000f;					
					break;
				}
				case 2:{
					//get North-south direction
					_info->NS = *p_dest;
					break;
				}
				case 3:{
					//get longitude
					sscanf((const char *)destBuf,"%3u%2u.%6u",&i32_ang,&i32_min,&i32_dec);
					_info->longd = (double)i32_ang+ ((double)i32_min + (double)i32_dec/1000000.000000f)/60.000000f;		
					break;
				}
				case 4:{
					//get East-west direction
					_info->EW = *p_dest;
					break;
				}
				case 5:{
					//get GPS quality indicator
					_info->gpsQ = *p_dest;
					break;
				}
				case 6:{
					//get Number of satellites in use
					sscanf((const char *)destBuf,"%2hhu",&_info->usedSatNum);
					break;
				}
				case 7:{
					//get HDOP
					sscanf((const char *)destBuf,"%3u.%2u",&i32_ang,&i32_dec);
					_info->f_hdop = (i32_ang*100+i32_dec)/100.0;					
					break;
				}
				case 8:{
					//get altitude
					sscanf((const char *)destBuf,"%d.%3u",&i32_ang,&i32_dec);
					if(*destBuf=='-'){
						if(i32_dec<10){
							_info->f_alt=(float)i32_ang-(float)i32_dec/10.0f;
						}
						else if((i32_dec>=10)&&(i32_dec<100)){
							_info->f_alt=(float)i32_ang-(float)i32_dec/100.0f;
						}
						else{
							_info->f_alt=(float)i32_ang-(float)i32_dec/1000.0f;
						}			
					}
					else{
						if(i32_dec<10){
							_info->f_alt=(float)i32_ang+(float)i32_dec/10.0f;
						}
						else if((i32_dec>=10)&&(i32_dec<100)){
							_info->f_alt=(float)i32_ang+(float)i32_dec/100.0f;
						}
						else{
							_info->f_alt=(float)i32_ang+(float)i32_dec/1000.0f;
						}
					}					
					break;
				}
			}			
		}	
		p_src++;		
	}
	return 1;
}

/*
	对RMC语句进行解析的函数，一些数据和GGA有重叠
*/
static uint8_t GPS_RMC(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA语句的一块中是否拥有数据。有数据时NMEA为两个逗号中间有数据例如‘,xxx,’，当没有数据的时候这块为‘,,’。 
	p_src = (uint8_t*)&_abuf[7];	

	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//这一块里有数据‘,xxx,’
		}	
		if(avail){	
			avail=0;
			switch(i){
				case 0:{
					//get time
					sscanf((const char *)destBuf,"%2hhu%2hhu%2hhu",&_info->Hour,&_info->Min,&_info->Sec);
					_info->Hour = (_info->Hour+8)%24;		 
					break;
				}
				case 1:{
					//get position system status
					if(*destBuf=='A'){
						_info->posEff = 1;
					}
					else{
						_info->posEff = 0;
					}					
					break;
				}
				case 2:{
					//get latitude 
					sscanf((const char *)destBuf,"%2u%2u.%6u",&i32_ang,&i32_min,&i32_dec);	
					_info->latd = (double)i32_ang+ ((double)i32_min + (double)i32_dec/1000000.000000f)/60.000000f;				
					break;
				}
				case 3:{
					//get North-south direction
					_info->NS = *destBuf;
					break;
				}
				case 4:{
					//get longitude
					sscanf((const char *)destBuf,"%3u%2u.%6u",&i32_ang,&i32_min,&i32_dec);	
					_info->longd = (double)i32_ang+ ((double)i32_min + (double)i32_dec/1000000.000000f)/60.000000f;						
					break;
				}
				case 5:{
					//get East-west direction
					_info->EW = *destBuf;
					break;
				}
				case 6:{
					//get knot speed
					sscanf((const char *)destBuf,"%u.%2u",&i32_ang,&i32_dec);
					_info->f_spdKts = (float)i32_ang + (float)i32_dec/100.00f;	
					break;
				}
				case 7:{
					//get course degree
					sscanf((const char *)destBuf,"%u.%2u",&i32_ang,&i32_dec);
					_info->f_trkDegT = (float)i32_ang + (float)i32_dec/100.00f;
					break;
				}
				case 8:{
					//get date
					sscanf((const char *)destBuf,"%2hhu%2hhu%2hhu",&_info->Day,&_info->Month,&_info->Year);	  
					break;
				}
			}
		}
		p_src++;
	}	
	
	return 2;
}

/*
	对GSV语句解析解析
*/
static uint8_t GPS_GSV(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA语句的一块中是否拥有数据。有数据时NMEA为两个逗号中间有数据例如‘,xxx,’，当没有数据的时候这块为‘,,’。 
	p_src = (uint8_t*)&_abuf[7];	

	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//这一块里有数据‘,xxx,’
		}	
		if(avail){	
			avail=0;
			switch(i){
				case 0:		//get total number of sentences
				case 1:{	//get Sentence number					
					break;
				}
				case 2:{	
#ifdef GPS_NMEA_V3
					if( (_abuf[1] == 'G') && (_abuf[2] == 'A') ){
						sscanf((const char*)destBuf,"%hhu",&vuNum[0]); 
					}
					else if( ((_abuf[1] == 'G') && (_abuf[2] == 'B')) || ((_abuf[1] == 'B') && (_abuf[2] == 'D'))){
						sscanf((const char*)destBuf,"%hhu",&vuNum[1]);
					}
					else if( (_abuf[1] == 'G') && (_abuf[2] == 'P') ){
						sscanf((const char*)destBuf,"%hhu",&vuNum[2]); 
					}
					else if( (_abuf[1] == 'G') && (_abuf[2] == 'Q') ){
						sscanf((const char*)destBuf,"%hhu",&vuNum[3]); 
					}
					else if( (_abuf[1] == 'I') && (_abuf[2] == 'R') ){
						sscanf((const char*)destBuf,"%hhu",&vuNum[4]); 
					}		
					else if( (_abuf[1] == 'G') && (_abuf[2] == 'L') ){
						sscanf((const char*)destBuf,"%hhu",&vuNum[5]); 
					}		
#endif
#ifdef GPS_NMEA_V4
					if(_abuf[2] == 'A'){
						if(_abuf[_len - 6] == '1'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[0]);
						}
						else if(_abuf[_len - 6] == '7'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[1]);
						}
					}
					else if(_abuf[2] == 'B'){
						if(_abuf[_len - 6] == '1'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[2]);
						}
						else if(_abuf[_len - 6] == '5'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[3]);
						}			
					}
					else if(_abuf[2] == 'P'){
						if(_abuf[_len - 6] == '1'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[4]);
						}
						else if(_abuf[_len - 6] == '8'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[5]);
						}				
					}
					else if(_abuf[2] == 'L'){
						sscanf((const char *)destBuf,"%hhu",&vuNum[6]);
					}
					else if(_abuf[2] == 'L'){
						if(_abuf[_len - 6] == '1'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[7]);
						}
						else if(_abuf[_len - 6] == '8'){
							sscanf((const char *)destBuf,"%hhu",&vuNum[8]);
						}		
					}		
#endif
					_info->viewSatNum = vuNum[0] + vuNum[1] + vuNum[2] + vuNum[3] 
													+vuNum[4]+ vuNum[5]+ vuNum[6]+ vuNum[7]+ vuNum[8] ;	
					break;
				}
			}			
		}
		p_src++;		
	}
	return 3;
}
