#include "uart_rx.h"
#include "stdio.h"
//缓冲区大小
#define BUFF_MAX_SIZE 128
//定义缓冲区 
uint8_t RxBuff[BUFF_MAX_SIZE]={0};//dma搬运数据到这里
uint8_t RxLine[BUFF_MAX_SIZE]={0};//cpu处理这里的数据
uint8_t RxReady=0;//判断是否接收数据完成
uint8_t RxLen=0;//计算接收数据长度

