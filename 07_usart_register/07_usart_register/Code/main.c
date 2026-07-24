#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
int main(void)//PB5,PE5
{
	USART_Init();

	while(1)
	{
		
		uint8_t ch=USART_ReciveChar();
		USART_SendChar(ch);
	}

}
