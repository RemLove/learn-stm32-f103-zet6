#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"usart.h"
#include"spi.h"
#include"flash.h"
uint8_t buff[100];
uint8_t flash_size=10;
int main(void)
{
	USART_Init();
	FLASH_Init();
	//2.测试
	FLASH_SectorErase(0x000000);
	FLASH_WaitNotBusy();
	for(uint8_t i=0;i<flash_size;i++)
	{
		buff[i]=i;
	}
	FLASH_PageWrite(0x000000,buff,flash_size);
	FLASH_WaitNotBusy();
	FLASH_PageRead(0x000000,buff,flash_size);
	for(uint8_t i=0;i<flash_size;i++)
	{
		printf("%d ",buff[i]);
	}
	while(1)
	{

	}

}
