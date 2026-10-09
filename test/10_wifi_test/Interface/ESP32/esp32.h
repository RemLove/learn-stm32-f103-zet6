#ifndef __ESP32_H__
#define __ESP32_H__

#include "usart.h"
#include <string.h>
void ESP32_Init(void);

void STM32SendToESP32(uint8_t *cmd, uint16_t size);

//每次stm32向esp32发送数据后，esp32会回应，stm32收到数据才证明发送数据成功
void ESP32Response(uint8_t *buff, uint16_t* size);//并不知道esp32回应的长度，所以需要传入一个指针，esp32回应的长度会写入这个指针


#endif /* __ESP32_H__ */

