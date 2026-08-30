#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
#include"string.h"
#include"m24c02.h"
#include"spi.h"
#include"flash.h"
uint8_t buff[100];
uint8_t size=10;
int main(void)//PB5,PE5
{
	USART_Init();
	FLASH_Init();
	//2.测试
	FLASH_SectorErase(0x000000);
	FLASH_WaitNotBusy();
	for(uint8_t i=0;i<size;i++)
	{
		buff[i]=i;
	}
	FLASH_PageWrite(0x000000,buff,size);
	FLASH_WaitNotBusy();
	FLASH_PageRead(0x000000,buff,size);
	for(uint8_t i=0;i<size;i++)
	{
		printf("%d ",buff[i]);
	}


	while(1)
	{

	}

}
