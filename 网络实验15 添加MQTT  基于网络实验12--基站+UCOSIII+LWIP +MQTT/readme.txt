实验器材:
	阿波罗STM32F7开发板--- H743基站5.0板子
	
实验目的:
	学会LWIP在STM32上的移植，本实验使用了UCOSIII操作系统,UDP通信。
	
硬件资源:
	1,DS0(连接在PB5) 
	2,串口1(波特率:115200,PD5/PD6连接在板载USB转串口芯片CH340上面) 
	3,STM32以太网MAC+外部PHY(LAN8720)
	4,网线一根
	
实验现象:
	本实验利用STM32的内部以太网MAC+外置PHY实现开发板的网络连接，本实验主要演示和讲解如何将LWIP和UCOSII
	操作系统结合起来实现开发板联网。本实验开启了LWIP的DHCP功能，开发板可以自动从路由器获取IP地址，也可
	以关闭DHCP，使用默认静态IP地址，默认静态IP地址为192.168.31.74，当开发板联网完成以后可以在电脑上向开
	发板发送ping包来测试网络是否通畅。

	默认静态IP地址为192.168.31.74
	电脑IP  192.168.31.5 
	通过串口查看获取DHCP过程。
	DS0周期闪烁，
	
	
	UDP端口8089，	每5秒发送一针，然后可以接收udp数据并打印。
	注意udp网络工具要核对好本地IP，不对的话禁止后重启。
	
	 
