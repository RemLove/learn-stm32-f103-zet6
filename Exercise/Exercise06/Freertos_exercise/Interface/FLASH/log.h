#ifndef __LOG_H
#define __LOG_H

#include <stdint.h>
//定长 16个字节
#define LOG_REC_SIZE   16u
//16 字节要塞下：序号(2) + 长度(1) + 数据 + 校验(1)
#define LOG_DATA_MAX   12u

void     Log_Init(void);                                /* 上电调一次 */
void     Log_Append(const uint8_t *data, uint8_t len);  /* 追加一条 */
uint16_t Log_Count(void);                               /* 已记多少条 */
void     Log_Read(uint16_t index, uint8_t *out16);      /* 读第 index 条 */

#endif
