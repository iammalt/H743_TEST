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
 ALIENTEK ������STM32H7������ ����ʵ��3
 WIP��UCOSIIIϵͳ��ֲ-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


OS_Q MQTT_Data_Queue;



//��LCD����ʾ��ַ��Ϣ����
//�������ȼ�
#define DISPLAY_TASK_PRIO	8
//�����ջ��С
#define DISPLAY_STK_SIZE	256
//������ƿ�
OS_TCB DisplayTaskTCB;
//�����ջ
CPU_STK	DISPLAY_TASK_STK[DISPLAY_STK_SIZE];
//������
void display_task(void *pdata);

//LED����
//�������ȼ�
#define LED_TASK_PRIO		9
//�����ջ��С
#define LED_STK_SIZE		256
//������ƿ�
OS_TCB LedTaskTCB;
//�����ջ
CPU_STK	LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);  

//START����
//�������ȼ�
#define START_TASK_PRIO		10
//�����ջ��С
#define START_STK_SIZE		512
//�����ջ
OS_TCB StartTaskTCB;
//�����ջ
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata); 

Device_Data_TypeDef  devive_data;
 
//��LCD����ʾ��ַ��Ϣ
//mode:1 ��ʾDHCP��ȡ���ĵ�ַ
//	  ���� ��ʾ��̬��ַ
void show_address(u8 mode)
{
//	u8 buf[30];
	if(mode==2)
	{
		printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);	
		printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
		printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
		
		//sprintf((char*)buf,"DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		//LCD_ShowString(30,130,210,16,16,buf); 
	//	sprintf((char*)buf,"DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
	//	LCD_ShowString(30,150,210,16,16,buf); 
	//	sprintf((char*)buf,"NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ
	//	LCD_ShowString(30,170,210,16,16,buf); 
	}
	else 
	{
		printf("Static IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);	
		printf("Static GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
		printf("NET MASK :%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
		
		//sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);						//��ӡ��̬IP��ַ
		//LCD_ShowString(30,130,210,16,16,buf); 
		//sprintf((char*)buf,"Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);	//��ӡ���ص�ַ
		//LCD_ShowString(30,150,210,16,16,buf); 
		//sprintf((char*)buf,"NET MASK :%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);	//��ӡ���������ַ
		//LCD_ShowString(30,170,210,16,16,buf); 
	}	
}

int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
    Write_Through();                //����ǿ��͸д��
    MPU_Memory_Protection();        //������ش洢����
    Cache_Enable();                 //��L1-Cache
    
	HAL_Init();				        //��ʼ��HAL��
	Stm32_Clock_Init(160,5,2,4);    //����ʱ��,400Mhz 
	delay_init(400);			    //��ʱ��ʼ��
	uart_init(115200);			    //���ڳ�ʼ��
	usmart_dev.init(200); 		    //��ʼ��USMART	
	LED_Init();				        //��ʼ��LED
	KEY_Init();				        //��ʼ������
	SDRAM_Init();                   //��ʼ��SDRAM
//LCD_Init();				        //��ʼ��LCD
	PCF8574_Init();                 //��ʼ��PCF8574
    my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMDTCM);		    //��ʼ��DTCM�ڴ��
//	POINT_COLOR = RED; 		                //��ɫ����
//	LCD_ShowString(30,30,200,20,16,"Apollo STM32H7");
//	LCD_ShowString(30,50,200,20,16,"LWIP+UCOSIII Test");
//	LCD_ShowString(30,70,200,20,16,"ATOM@ALIENTEK");
//	LCD_ShowString(30,90,200,20,16,"2018/7/12");
//    
	OSInit(&err); 					//UCOSIII��ʼ��
	while(lwip_comm_init()) 		//lwip��ʼ��
	{
		printf("Lwip Init failed!");
//		LCD_ShowString(30,110,200,20,16,"Lwip Init failed!"); 	//lwip��ʼ��ʧ��
		delay_ms(500);
//		LCD_Fill(30,110,230,150,WHITE);
//		delay_ms(500); 
	}
	printf("Lwip Init Success!");
	mqtt_thread_init();
//	while() 										//��ʼ��udp_demo(����udp_demo�߳�)
//	{
	//	LCD_ShowString(30,150,200,20,16," failed!!"); 		//udp����ʧ��
//		printf("mqtt failed!");
//		delay_ms(500);
	//	LCD_Fill(30,150,230,170,WHITE);
//		delay_ms(500);
//	}
//	LCD_ShowString(30,110,200,20,16,"Lwip Init Success!"); 		//lwip��ʼ���ɹ�
	printf("mqtt Success!");
	OS_CRITICAL_ENTER();//�����ٽ���x
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err); 		//����UCOS	  
}

//start����
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
#endif

	OS_CRITICAL_ENTER();	//�����ٽ���
	//����LED����
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
				 
	//������ʾ����
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
				 			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�����ٽ���
}

//��ʾ��ַ����Ϣ
void display_task(void *pdata)
{
	OS_ERR err;
	while(1)
	{ 
#if LWIP_DHCP									//������DHCP��ʱ��
		if(lwipdev.dhcpstatus != 0) 			//����DHCP
		{
			show_address(lwipdev.dhcpstatus );	//��ʾ��ַ��Ϣ
			OS_TaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);		//������������	
		}
#else
		show_address(0); 						//��ʾ��̬��ַ
		OS_TaskSuspend((OS_TCB*)&DisplayTaskTCB,&err);		//������������	
#endif //LWIP_DHCP
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
	}
}

//led����
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
		OSTimeDlyHMSM(0,0,0,10000,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ500ms
 	}
}
