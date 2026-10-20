#ifndef __LOG_H
#define __LOG_H

#include "flash.h"
//十六个字节
#define LOG_REC_SIZE 16
//上电
void Log_Init(void);
//追加一条
void Log_Append(const uint8_t *data, uint8_t len);
#endif
