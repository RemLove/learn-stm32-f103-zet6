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
				socket(SOCKET_NUM,Sn_MR_UDP,src_port,0); // 打开UDP套接字
				break;
			case SOCK_UDP:{
				rxlen=getSn_SR(SOCKET_NUM);
				if(rxlen)
				{
					recvfrom(SOCKET_NUM,rxbuffer,rxlen,dest_ip,&dest_port);
					sendto(SOCKET_NUM,rxbuffer,rxlen,dest_ip,dest_port);
					memset(rxbuffer,0,rxlen);
				}
				break;
			}
			default:
				break;
		}
	}

}
