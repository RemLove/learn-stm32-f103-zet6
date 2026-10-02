#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"usart.h"
#include"spi.h"
#include"flash.h"
#include"eth.h"

uint8_t buff[100];
uint8_t flash_size=10;
int main(void)
{
	ETH_Init();
	
	while(1)
	{

	}

}
