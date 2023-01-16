#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h"
#include "key.h"
#include "device.h"
#include "lcd.h"
#include "sdram.h"
#include "malloc.h"
#include "usmart.h"
#include "pcf8574.h"
#include "mpu.h"
#include "timer.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/netif.h"
#include "ethernetif.h"
#include "lwipopts.h"
#include "includes.h"
//#include "udp_demo.h"
//#include "tcp_client_demo.h"
#include "mqtt_client_demo.h"
/************************************************
 ALIENTEK 阿波罗STM32H7开发板 网络实验3
 WIP带UCOSIII系统移植-HAL库函数版
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


OS_Q MQTT_Data_Queue;



//在LCD上显示地址信息任务
//任务优先级
#define DISPLAY_TASK_PRIO	8
//任务堆栈大小
#define DISPLAY_STK_SIZE	256
//任务控制块
OS_TCB DisplayTaskTCB;
//任务堆栈
CPU_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];
//任务函数
void display_task(void *pdata);

//LED任务
//任务优先级
#define LED_TASK_PRIO		9
//任务堆栈大小
#define LED_STK_SIZE		256
//任务控制块
OS_TCB LedTaskTCB;
//任务堆栈
CPU_STK	LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);  

//START任务
//任务优先级
#define START_TASK_PRIO		10
//任务堆栈大小
#define START_STK_SIZE		512
//任务堆栈
OS_TCB StartTaskTCB;
//任务堆栈
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata); 

Device_Data_TypeDef  devive_data;
 
//在LCD上显示地址信息
//mode:1 显示DHCP获取到的地址
//	  其他 显示静态地址
void show_address(u8 mode)
{
//	u8 buf[30];
	if(mode==2)
	{
		printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);	
		printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
		printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
		
		//sprintf((char*)buf,"DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		//LCD_ShowString(30,130,210,16,16,buf); 
	//	sprintf((char*)buf,"DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
	//	LCD_ShowString(30,150,210,16,16,buf); 
	//	sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
	//	LCD_ShowString(30,170,210,16,16,buf); 
	}
	else 
	{
		printf("Static IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);	
		printf("Static GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
		printf("NET MASK :%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
		
		//sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//打印动态IP地址
		//LCD_ShowString(30,130,210,16,16,buf); 
		//sprintf((char*)buf,"Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//打印网关地址
		//LCD_ShowString(30,150,210,16,16,buf); 
		//sprintf((char*)buf,"NET MASK :%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//打印子网掩码地址
		//LCD_ShowString(30,170,210,16,16,buf); 
	}	
}

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
    Write_Through();                //开启强制透写！
    MPU_Memory_Protection();        //保护相关存储区域
    Cache_Enable();                 //打开L1-Cache
    
	HAL_Init();				        //初始化HAL库
	Stm32_Clock_Init(160,5,2,4);    //设置时钟,400Mhz 
	delay_init(400);			    //延时初始化
	uart_init(115200);			    //串口初始化
	usmart_dev.init(200); 		    //初始化USMART	
	LED_Init();				        //初始化LED
	KEY_Init();				        //初始化按键
	SDRAM_Init();                   //初始化SDRAM
//LCD_Init();				        //初始化LCD
	PCF8574_Init();                 //初始化PCF8574
    my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMDTCM);		    //初始化DTCM内存池
//	POINT_COLOR = RED; 		                //红色字体
//	LCD_ShowString(30,30,200,20,16,"Apollo STM32H7");
//	LCD_ShowString(30,50,200,20,16,"LWIP+UCOSIII Test");
//	LCD_ShowString(30,70,200,20,16,"ATOM@ALIENTEK");
//	LCD_ShowString(30,90,200,20,16,"2018/7/12");
//    
	OSInit(&err); 					//UCOSIII初始化
	while(lwip_comm_init()) 		//lwip初始化
	{
		printf("Lwip Init failed!");
//		LCD_ShowString(30,110,200,20,16,"Lwip Init failed!"); 	//lwip初始化失败
		delay_ms(500);
//		LCD_Fill(30,110,230,150,WHITE);
//		delay_ms(500); 
	}
	printf("Lwip Init Success!");
	mqtt_thread_init();
//	while() 										//初始化udp_demo(创建udp_demo线程)
//	{
	//	LCD_ShowString(30,150,200,20,16," failed!!"); 		//udp创建失败
//		printf("mqtt failed!");
//		delay_ms(500);
	//	LCD_Fill(30,150,230,170,WHITE);
//		delay_ms(500);
//	}
//	LCD_ShowString(30,110,200,20,16,"Lwip Init Success!"); 		//lwip初始化成功
	printf("mqtt Success!");
	OS_CRITICAL_ENTER();//进入临界区x
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err); 		//开启UCOS	  
}

//start任务
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //创建DHCP任务
#endif

	OS_CRITICAL_ENTER();	//进入临界区
	//创建LED任务
	OSTaskCreate((OS_TCB 	* )&LedTaskTCB,		
				 (CPU_CHAR	* )"led0 task", 		
                 (OS_TASK_PTR )led_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LED_TASK_PRIO,     
                 (CPU_STK   * )&LED_TASK_STK[0],	
                 (CPU_STK_SIZE)LED_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
				 
	//创建显示任务
	OSTaskCreate((OS_TCB 	* )&DisplayTaskTCB,		
				 (CPU_CHAR	* )"display task", 		
                 (OS_TASK_PTR )display_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )DISPLAY_TASK_PRIO,     	
                 (CPU_STK   * )&DISPLAY_TASK_STK[0],	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE/10,	
                 (CPU_STK_SIZE)DISPLAY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);
				 			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}

//显示地址等信息
void display_task(void *pdata)
{
	OS_ERR err;
	while(1)
	{ 
#if LWIP_DHCP									//当开启DHCP的时候
		if(lwipdev.dhcpstatus != 0) 			//开启DHCP
		{
			show_address(lwipdev.dhcpstatus );	//显示地址信息
			OS_TaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);		//挂起自身任务	
		}
#else
		show_address(0); 						//显示静态地址
		OS_TaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);		//挂起自身任务	
#endif //LWIP_DHCP
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
	}
}

//led任务
void led_task(void *pdata)
{
	OS_ERR err;
	Device_Data_TypeDef * send_device_data;
	while(1)
	{
		LED0_Toggle;
		printf("LED0  Toggle !");	
		//tcp_client_flag |= LWIP_SEND_DATA;
		send_device_data->temperature=55.5;	
		send_device_data->humidity=99.9;			
		sys_mbox_post(&MQTT_Data_Queue,send_device_data);		
		OSTimeDlyHMSM(0,0,0,10000,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
 	}
}
