#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"usart.h"
#include"spi.h"
#include"flash.h"
#include "fsmc.h"
uint8_t buff[100];
uint8_t flash_size=10;

uint8_t v1 __attribute__((at(0x68000000)));

int main(void)
{
	USART_Init();
	FLASH_Init();
	FSMC_Init();
	printf("v1=%d,@%p\r\n",v1,&v1);
	while(1)
	{

	}

}
