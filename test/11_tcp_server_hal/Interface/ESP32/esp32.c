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
    //4.把重启期间吐出来的启动日志读掉，免得混进下一条命令的响应里
    ESP32Response(rxBuff,&rxSize);
}

void STM32SendToESP32(uint8_t *cmd, uint16_t size)
{
    uint8_t retry = 0;

    //清空缓冲区
    memset(rxBuff,0,1024);
    rxSize = 0;
    //1.发送命令
    if(HAL_UART_Transmit(&huart2, cmd, size, 1000)!=HAL_OK)
    {
        printf("TX fail\n");
        return;
    }
    //2.读取响应
    //  HAL_UARTEx_ReceiveToIdle() 是阻塞版：收到 ESP32 一帧结束(IDLE)或超时才返回，
    //  所以调一次就能拿到完整响应。它每次进来都会把 rxSize 清零，
    //  所以不能"先收一次、再判OK"，必须一边收一边判。
    //esp32可能也会主动发一些报告 比如状态改变，发送忙 等等
    //需要判断响应里面是否有OK
    do
    {
        ESP32Response(rxBuff,&rxSize);
        if(rxSize==0) break;        //这一轮1秒内啥都没收到，ESP32多半不在线，别再死等
    } while (strstr((char*)rxBuff,"OK")==NULL && (++retry)<5);

    if(rxSize!=0)
        printf("%.*s\n",(int)rxSize,(char*)rxBuff);
    else
        printf("no response\n");
}

void ESP32Response(uint8_t *buff, uint16_t* size)
{
    //串口接收，因为是esp32回应，串口并不知道要接收多少字节，是接收变长的数组
    //第3个参数是【缓冲区容量】(最多能收多少)，不是要收的长度——写成变量很容易踩坑
    //第4个参数才是【实际收到多少字节】的输出
    *size = 0;
    HAL_UARTEx_ReceiveToIdle(&huart2,buff,1024,size,1000);
}
