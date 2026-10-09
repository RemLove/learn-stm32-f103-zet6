#include "esp32.h"

uint8_t rxBuff[1024]={0};
uint16_t rxSize;

void ESP32_Init(void)
{
    //1.初始化uart2
    MX_USART2_UART_Init();
    //2.初始化esp32 发送命令集 
    uint8_t *cmd="AT+RST\r\n";
    STM32SendToESP32(cmd, strlen((char*)cmd));
    //3.重启需要延迟
    HAL_Delay(2000);

}

void STM32SendToESP32(uint8_t *cmd, uint16_t size)
{
    //清空缓冲区
    memset(rxBuff,0,1024);
    //1.发送命令
    HAL_UART_Transmit(&huart2, cmd, size, 1000);
    //2.读取响应
    ESP32Response(rxBuff,&size);
    //esp32可能也会主动发一些报告 比如状态改变，发送忙 等等
    //需要判断响应里面是否有OK
    
    do
    {
        ESP32Response(rxBuff,&rxSize);
    } while (strstr((char*)rxBuff,"OK")==NULL);
    printf("%.*s\n",rxSize,rxBuff);
}

void ESP32Response(uint8_t *buff, uint16_t* size)
{
    //串口接收，因为是esp32回应，串口并不知道要接收多少字节，是接收变长的数组
    HAL_UARTEx_ReceiveToIdle(&huart2,buff,rxSize,size,1000);
}
