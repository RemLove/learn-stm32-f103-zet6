#ifndef __UART_RX_H
#define __UART_RX_H

#include <stdint.h>

//缓冲区大小
#define BUFF_MAX_SIZE 128

//dma接收缓冲区(缓冲数组交给DMA/HAL，不需要volatile；用下面的volatile标志做跨上下文同步)
extern uint8_t RxBuff[BUFF_MAX_SIZE];//dma搬运数据到这里
extern uint8_t RxLine[BUFF_MAX_SIZE];//cpu处理这里的数据
extern volatile uint8_t RxReady;//判断是否接收数据完成
extern volatile uint8_t RxLen;//计算接收数据长度
extern volatile uint8_t RxErr;//dma接收出错标志(中断里置位，主循环里处理)

//解析数据
void parse_and_execute(char *rx_line);

#endif
