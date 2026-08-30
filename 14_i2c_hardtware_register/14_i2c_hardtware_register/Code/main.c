#include"stm32f10x.h"
#include"led.h"
#include"delay.h"
#include"key.h"
#include"usart.h"
#include"string.h"
#include"m24c02.h"
uint8_t buff[100]={0};
uint8_t size=0;
int main(void)//PB5,PE5
{
	USART_Init();
	M24C02_Init();
	// uint8_t *str="hello world";
	// USART_SendString(str,strlen((char*)str));
	//printf("Satella\n");
	M24C02_WriteByte(0x00,'a');
	M24C02_WriteByte(0x01,'b');
	M24C02_WriteByte(0x02,'c');
	uint8_t byte1= M24C02_ReadByte(0x00);
	uint8_t byte2= M24C02_ReadByte(0x01);
	uint8_t byte3= M24C02_ReadByte(0x02);
	printf("%c ",byte1);
	printf("%c ",byte2);
	printf("%c ",byte3);

	M24C02_WriteBytes(0x00,"123456",6);
	uint8_t buffer[100]={0};
	M24C02_ReadBytes(0x00,buffer,6);
	printf("%s",buffer);
	while(1)
	{

	}

}
