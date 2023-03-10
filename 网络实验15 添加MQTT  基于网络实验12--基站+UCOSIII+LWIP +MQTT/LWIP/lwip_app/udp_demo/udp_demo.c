#include "udp_demo.h"
#include "lwip_comm.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4&F7&H7开发板
//NETCONN API编程方式的UDP测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/8/5
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 
//UDP任务
#define UDP_PRIO		6
//任务堆栈大小
#define UDP_STK_SIZE	300
//任务控制块
OS_TCB	UdpTaskTCB;
//任务堆栈
CPU_STK UDP_TASK_STK[UDP_STK_SIZE];


u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区
//UDP发送数据内容
const u8 *udp_demo_sendbuf="Apollo STM32F4/F7/H7 NETCONN UDP demo send data\r\n";
u8 udp_flag;							//UDP数据发送标志位

//udp任务函数
static void udp_thread(void *arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	static struct netconn *udpconn;
	static struct netbuf  *recvbuf;
	static struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u32 data_len = 0;
	struct pbuf *q;
	
	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP);  //创建一个UDP链接
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //创建UDP连接成功
	{
		err = (OS_ERR)netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //构造目的IP地址
        netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//连接到远端主机
		if(err == ERR_OK)//绑定完成
		{
			while(1)
			{
				if((udp_flag & LWIP_SEND_DATA) == LWIP_SEND_DATA) //有数据要发送
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf,strlen((char *)udp_demo_sendbuf));
					memcpy(sentbuf->p->payload,(void*)udp_demo_sendbuf,strlen((char*)udp_demo_sendbuf));
					err = (OS_ERR)netconn_send(udpconn,sentbuf);  	//将netbuf中的数据发送出去
					if(err != ERR_OK)
					{
						printf("发送失败\r\n");
						netbuf_delete(sentbuf);      //删除buf
					}
					udp_flag &= ~LWIP_SEND_DATA;	//清除数据发送标志
					netbuf_delete(sentbuf);      	//删除buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //接收数据
				if(recvbuf != NULL)          	//接收到数据
				{ 
					OS_CRITICAL_ENTER();		//进入临界区
					memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
					for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
					{
						//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
						//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
						if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
						else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
					}
					OS_CRITICAL_EXIT();					//退出临界区
					data_len=0;  //复制完成后data_len要清零。
					printf("%s\r\n",udp_demo_recvbuf);  //打印接收到的数据
					netbuf_delete(recvbuf);      		//删除buf
				}else OSTimeDlyHMSM(0,0,0,5,OS_OPT_TIME_HMSM_STRICT,&err); //延时5ms
			}
		}else printf("UDP绑定失败\r\n");
	}else printf("UDP连接创建失败\r\n");
}

//创建UDP线程
//返回值:0 UDP创建成功
//		其他 UDP创建失败
u8 udp_demo_init(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER();//进入临界区
	//创建UDP任务
	OSTaskCreate((OS_TCB 	* )&UdpTaskTCB,		
				 (CPU_CHAR	* )"udp task", 		
                 (OS_TASK_PTR )udp_thread, 			
                 (void		* )0,					
                 (OS_PRIO	  )UDP_PRIO,     
                 (CPU_STK   * )&UDP_TASK_STK[0],	
                 (CPU_STK_SIZE)UDP_STK_SIZE/10,	
                 (CPU_STK_SIZE)UDP_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	OS_CRITICAL_EXIT();	//退出临界区
	return err;
}

