#include "wifi.h"

//定义两个内部函数 设置ESP32 WIFI模式
static void WIFI_SET_STA(void);
static void WIFI_SET_AP(void);

void WIFI_Init(WIFI_MODE mode)
{
    //1.初始化esp32
    ESP32_Init();
    //2.设置模式
    if(mode==STA)
    {
        WIFI_SET_STA();
    }
    else if(mode==AP)
    {
        WIFI_SET_AP();
    }
}

static void WIFI_SET_STA(void)
{
    //1.设置模式
    uint8_t* cmd="AT+CWMODE=1\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //2.设置ip
    cmd="AT+CWJAP=\"越王\",\"113937604\"\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //3.查询esp32 ip   会自动分配，我们只用查询
    cmd="AT+CIPAP?\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
}

static void WIFI_SET_AP(void)
{
    //1.设置模式
    uint8_t* cmd="AT+CWMODE=2\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //2.设置ip
    cmd="AT+CWSAP=\"Satella\",\"113937604\",5,3\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //3.设置ip
    cmd="AT+CIPAP=\"192.168.8.1\"\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
}
