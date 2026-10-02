#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"usart.h"
#include"spi.h"
#include"flash.h"
#include "lcd.h"
uint8_t buff[100];
uint8_t flash_size=10;

int main(void)
{
	USART_Init();
	FLASH_Init();
	LCD_Init();
	uint32_t id=LCD_ReadId();
	printf("%#x\r\n",id);

	//开背光，全屏刷白色
	LCD_BGOn();
	LCD_ClearAll(WHITE);

	//LCD_WritePicture(0,0);

	//LCD_DrowPoint(100,100,5,BLUE);

	LCD_DrawLine(100,100,200,200,2,RED);

	LCD_DrawCFX(0,0,100,100,2,RED);
	LCD_DrawCircle(100,100,50,2,RED);
	LCD_DrawCircle_Pro(150,150,50,2,RED,RED);
	while(1)
	{

	}

}
