#include "rtthread.h"
#include "app_rt_thread.h"
#include "stdio.h"
#include "main.h"

struct rt_thread led_thread;
rt_thread_t gps_thread;
rt_uint8_t rt_led_thread_stack[128];
void led_task_entry(void *parameter);
void gps_th_entry(void *parameter);

struct rt_semaphore gps_sem;
 
//初始化线程函数
void MX_RT_Thread_Init(void)
{		
		
	driver_init();
	
	//初始化LED线程
	rt_thread_init(&led_thread,"led",led_task_entry,RT_NULL,&rt_led_thread_stack[0],sizeof(rt_led_thread_stack),3,20);
	//开启线程调度
	rt_thread_startup(&led_thread);
	
	//初始化信号量	
	rt_sem_init(&gps_sem, "gps_sem", 0, RT_IPC_FLAG_FIFO);	
	//初始化GPS线程
	gps_thread = rt_thread_create("gps_th", gps_th_entry, NULL, 512, 3, 20);
	rt_thread_startup(gps_thread);
	
	   
}

 
//LED任务
 void led_task_entry(void *parameter)
{
	while(1)
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
		rt_thread_delay(1000);
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1, GPIO_PIN_RESET);
		rt_thread_delay(1000);
	}
}

//解析NMEA语句的线程
int apple;
GPS_INFO gps_info;
void gps_th_entry(void *parameter)
{
	GPS_Open();
	GPS_Control(GPS_WORKMODE_INFO);
	while(1){
		rt_sem_take(&gps_sem, RT_WAITING_FOREVER); 
		GPS_GetData(&gps_info ,NULL, 0); 
		apple++;
	}
}
