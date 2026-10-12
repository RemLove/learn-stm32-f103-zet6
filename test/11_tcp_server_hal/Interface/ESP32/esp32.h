#ifndef __ESP32_H__
#define __ESP32_H__

#include "usart.h"
#include <string.h>
#include <stdio.h>

/* AT 命令响应超时：普通命令 */
#define ESP32_CMD_TIMEOUT_MS    1000u
/* 连接类命令（AT+CWJAP / AT+CIPSERVER ...）要等好几秒，必须单独给长超时 */
#define ESP32_LONG_TIMEOUT_MS   8000u

void ESP32_Init(void);

/* 发命令 + 等响应（用默认超时） */
void STM32SendToESP32(uint8_t *cmd, uint16_t size);
/* 发命令 + 自定义超时，给 AT+CWJAP 这种慢命令用 */
void STM32SendToESP32Ex(uint8_t *cmd, uint16_t size, uint32_t timeout_ms);

/* 等某个关键字出现（"OK" / "ERROR" / ">" ），把等到的字节一起带回 buff，
   返回带回的字节数；超时返回已收到的字节数（可能为 0） */
uint16_t ESP32_RxWait(const char *token, uint8_t *buff, uint16_t cap, uint32_t timeout_ms);

/* 单点接收的环形缓冲：非阻塞地从里面取数据 */
uint16_t ESP32_RxRead(uint8_t *dst, uint16_t len);
uint16_t ESP32_RxAvail(void);
void     ESP32_RxFlush(void);

/* 最近一次命令响应的原文，留着给 printf 观察用 */
extern uint8_t  rxBuff[1024];
extern uint16_t rxSize;

#endif /* __ESP32_H__ */
