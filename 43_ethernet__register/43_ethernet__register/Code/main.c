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
int main(void)
{
	USART_Init();   // 串口1：115200，printf 由此输出
	ETH_Init();
	
	while(1)
	{
		UDP_Start();
		UDP_ReceiveData(rxbuffer,&rxlen,src_ip,&src_port);
		// 只有真收到数据才回传，避免 len=0 触发空包 SEND
		if(rxlen > 0)
		{
			
			UDP_SendData(rxbuffer,rxlen,src_ip,src_port);
			rxlen=0;
		}
	}

}
