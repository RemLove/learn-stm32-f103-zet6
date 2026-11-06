#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"usart.h"
#include"spi.h"
#include"flash.h"
#include"eth.h"
#include"TCP.h"
#include"UDP.h"
uint8_t rxbuffer[1024];
uint16_t rxlen=0;

uint8_t src_ip[4]={192,168,1,100};
uint16_t src_port=9999;

uint8_t buff[100];
uint8_t flash_size=10;


uint8_t dest_ip[4]={0,0,0,0};
uint8_t dest_mac[6]={0,0,0,0,0,0};
uint16_t dest_port=0;


#define SOCKET_NUM 0
int main(void)
{
	USART_Init();   // 串口1：115200，printf 由此输出
	ETH_Init();
	
	while(1)
	{
		switch(getSn_SR(SOCKET_NUM))
		{
			case SOCK_CLOSED:
				//printf("SOCK_CLOSED\r\n");
				socket(SOCKET_NUM,Sn_MR_TCP,src_port,Sn_MR_ND); // 打开套接字
				break;
			case SOCK_INIT:
				//printf("SOCK_INIT\r\n");
				listen(SOCKET_NUM); // 监听套接字
				break;
			case SOCK_ESTABLISHED:
				//printf("SOCK_ESTABLISHED\r\n");
				if(getSn_IR(SOCKET_NUM)&Sn_IR_CON)
				{
					setSn_IR(SOCKET_NUM,Sn_IR_CON); // 清除连接建立中断标志
					//看一下谁连的
					getSn_DHAR(SOCKET_NUM,dest_mac); // 读取远程MAC
					getSn_DIPR(SOCKET_NUM,dest_ip); // 读取远程IP
					dest_port =getSn_DPORT(SOCKET_NUM); // 读取远程端口
					printf("remote MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", dest_mac[0], dest_mac[1], dest_mac[2], dest_mac[3], dest_mac[4], dest_mac[5]);
					printf("connect from %d.%d.%d.%d:%d\r\n",dest_ip[0],dest_ip[1],dest_ip[2],dest_ip[3],dest_port);
					printf("port %d connected\r\n",dest_port);
				}
				break;
			case SOCK_CLOSE_WAIT:
				close(SOCKET_NUM); // 断开连接
				break;
			default:
				break;
		}
	}

}
