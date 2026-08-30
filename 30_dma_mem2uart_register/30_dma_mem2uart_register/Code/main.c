#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
#include"string.h"
#include"stdio.h"
#include"dma.h"
uint8_t srcBuff[10]="123456789";
int main(void)
{
	USART_Init();
	DMA1_Init();
	DMA1_Transfer((uint32_t)srcBuff,(uint32_t)&USART1->DR,10);

	while(1)
	{
		
	}

}
