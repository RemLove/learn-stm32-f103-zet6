#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
#include"string.h"
uint8_t buff[100]={0};
uint8_t size=0;
int main(void)//PB5,PE5
{
	USART_Init();
	// uint8_t *str="hello world";
	// USART_SendString(str,strlen((char*)str));

	while(1)
	{

	}

}
