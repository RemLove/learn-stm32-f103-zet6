#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
#include"string.h"
#include"stdio.h"
#include"dma.h"
volatile uint8_t flag=0;
uint8_t srcBuff[10]="123456789";
uint8_t dstBuff[10];
int main(void)
{
	DMA1_Init();
	USART_Init();
	DMA1_Transfer((uint32_t)srcBuff,(uint32_t)dstBuff,10);

	while(1)
	{
		if(flag==1)
		{
			printf("DMA传输完成\r\n");
			flag=0;
			for(int i=0;i<10;i++)
			{
				printf("%c",dstBuff[i]);
			}
		}
	}

}
