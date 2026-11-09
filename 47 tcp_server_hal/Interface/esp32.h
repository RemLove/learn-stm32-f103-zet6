#ifndef __ESP32_H
#define __ESP32_H
//复位一下
void ESP32_Init(void);
//发送命令
void ESP32_SendCmd(uint8_t *cmd, uint16_t len);
//接收命令
void ESP32_ReceiveCmd(uint8_t *cmd, uint16_t* len);
#endif
