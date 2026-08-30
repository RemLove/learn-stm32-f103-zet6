#include"stm32f10x.h"
#include"led.h"
#include"systick.h"
int main(void)//PB5,PE5
{
	LED_Init();
	Systick_Init();


	while(1)
	{

	}

}
