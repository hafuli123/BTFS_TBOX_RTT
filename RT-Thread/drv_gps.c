#include "drv_gps.h" 

GPS_VAR gps_var;

uint8_t gpsRxBuf[GPS_RX_MAXLEN] , gpsSavBuf[GPS_RX_MAXLEN];
__IO uint8_t gpsRxStep=0;
__IO uint16_t rxCnt=0;		/* �ѽ��յ����ֽ��� */
static __IO uint16_t gpsRxLen=0;
static __IO uint8_t sSum, ucSum; /* У��NMEA���ʱʹ�� */
static __IO uint32_t i32_ang, i32_min, i32_dec;
static __IO uint8_t vuNum[9];
static __IO uint8_t wMode;

/* 
	GPS�Ĵ��ڽ����жϷ�����
*/
static uint8_t data;int bana;
//extern struct rt_semaphore gps_sem;
void GPS_IRQHANDLER(void)
{
	if((__HAL_UART_GET_FLAG(&gps_var.gps_huart,UART_FLAG_RXNE)!= RESET) ||(__HAL_UART_GET_FLAG(&gps_var.gps_huart, UART_FLAG_ORE) != RESET)){
		data=READ_REG(gps_var.gps_huart.Instance->DR); // ��ȡ����
		/* ��������ж� */
		if(__HAL_UART_GET_FLAG(&gps_var.gps_huart, UART_FLAG_ORE) != RESET)
		{
			/* �������ж� */
			__HAL_UART_CLEAR_FLAG(&gps_var.gps_huart, UART_FLAG_ORE);
		}	
		
		/* �յ�NMEA���Ŀ�ͷ֡$ */		
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
				/* ��Ϊ2���ɼ�����ͣס���ȴ��ɼ����ؿ� */
				/* ��ʱ������һֱ������һ��NMEA��� */
				/* GPS_GetData()�������һ��NMEA�����ռ����������������gpsRxStep��Ϊ0���ж������ɿ����µ�һ���ɼ� */
				gpsRxStep=2;
				/* �����ź���������������߳̿������� */
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
	GPSģ�鿪��
*/
void GPS_Open(void){
	gps_var.gps_init();
}

/*
	GPSģ��ر�
*/
void GPS_Close(void){
	gps_var.gps_disinit();
}

/*
	��ȡ���ݣ������ֹ���ģʽ��ȡ��ͬ���������ݣ�

	Ĭ�Ϸ�ʽ0�����϶Բɼ�����NMEA�����н������ѽ������GPS_INFO�ṹ���У�Get GPS_INFO Data����ȡGPS���ݣ�
						 �������GPS_INFO�ṹ�壬�ѽ��������Ϣ���ݴ���ṹ���С�
	��ʽ1�����������ֱ�Ӹ�����������յ���һ��NMEA��䣬Get NMEA Data����ȡ�յ���NMEA��䡣
				 �������_rbuf����������NMEA���Ļ�������ע��_rbuf�Ļ�������С��Ҫ����100��_freshΪ�����������_rbuf���Ƿ�ˢ�£����жϲɼ�һ���µ�NMEA��䣬0��ˢ�¡�

	����ֵ��
	ģʽ0ʱ������0������ǰ���������NMEA�����Ч��û��������Ϣ������1��2��3�ֱ�����ոս�����������NMEA��䣬GGA��RMC��GSV
	ģʽ1ʱ������ֵ1��������NMEA���ĳ���
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
			if(!_fresh){	//�Ƿ���Ҫˢ�£�����ʼ���²ɼ���һ��NMEA��䣬0�����ˢ��
				gpsRxStep=0;
			}
			return _len;
		}
	}
	return 0;
}

/*
	��GPS�����ݲɼ�����ѡ��ѡ����Ҫ�����������
*/
void GPS_Control(uint8_t _wmod)
{
	wMode = _wmod;
}

/*
	��һ��NMEA������У�顣
	����ֵ��У��ʧ�ܷ���0��У����ȷ����1.
*/
static uint8_t GPS_ChkXor(uint8_t *_rbuf,uint16_t _len)
{
	sSum=0;
	ucSum=0;
	if(_len<3){
		return 0;
	}
	/* ���û��У���ֽڣ�Ҳ��Ϊ���� */
	if(_rbuf[_len-5]!='*'){
		return 0;
	}	
	
	for(int i = 1; i < _len - 5; i++){
		/* ��������ַ�ASCII�ַ� */
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
	��NMEA����ID���з��࣬��ͬ������ִ�в�ͬ�Ľ�������
	����ֵ
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
	��GGA�����н����ĺ���
*/
static uint8_t GPS_GGA(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA����һ�����Ƿ�ӵ�����ݡ�������ʱNMEAΪ���������м����������确,xxx,������û�����ݵ�ʱ�����Ϊ��,,���� 
	p_src = (uint8_t*)&_abuf[7];	
	
	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//��һ���������ݡ�,xxx,��
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
	��RMC�����н����ĺ�����һЩ���ݺ�GGA���ص�
*/
static uint8_t GPS_RMC(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA����һ�����Ƿ�ӵ�����ݡ�������ʱNMEAΪ���������м����������确,xxx,������û�����ݵ�ʱ�����Ϊ��,,���� 
	p_src = (uint8_t*)&_abuf[7];	

	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//��һ���������ݡ�,xxx,��
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
	��GSV����������
*/
static uint8_t GPS_GSV(uint8_t * _abuf , uint16_t _len , GPS_INFO *_info)
{
	uint8_t *p_src,*p_dest , destBuf[20];
	uint8_t avail;	//NMEA����һ�����Ƿ�ӵ�����ݡ�������ʱNMEAΪ���������м����������确,xxx,������û�����ݵ�ʱ�����Ϊ��,,���� 
	p_src = (uint8_t*)&_abuf[7];	

	for(int i=0;i<9;i++){
		p_dest = destBuf;
		while(*p_src!=','){
			*p_dest = *p_src;
			p_dest++;p_src++;
			avail=1;	//��һ���������ݡ�,xxx,��
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
