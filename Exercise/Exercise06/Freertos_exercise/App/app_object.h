/* 当前文件职责：数据结构定义和信号量/互斥量/队列/事件标志组 声明 */
#ifndef __APP_OBJECT_H
#define __APP_OBJECT_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* 数据结构定义 */
typedef struct
{
    uint32_t seq;       /* 全局序号：丢帧表现为 seq 缺口 */
    uint32_t tick_ms;   /* 采集时刻的系统 tick */
    uint16_t raw_adc;   /* 0~4095 */
    uint16_t mv;        /* 毫伏值 */
}sample_t;


#endif /* __APP_OBJECT_H */

