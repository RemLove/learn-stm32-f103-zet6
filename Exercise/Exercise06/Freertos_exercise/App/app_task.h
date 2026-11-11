/* 当前文件职责：宏定义任务优先级、栈大小、任务句柄和全局唯一出口 */
#ifndef __APP_TASK_H
#define __APP_TASK_H

#include "FreeRTOS.h"
#include "task.h"
/* 五个任务优先级 acquire cmdd display storrage monitor */
#define TASK_PRIO_ACQUIRE   ( tskIDLE_PRIORITY + 7 )   /* 采样窗口错过 = 数据永久丢失 */
#define TASK_PRIO_COMM      ( tskIDLE_PRIORITY + 5 )   /* 命令响应要跟手 */
#define TASK_PRIO_DISPLAY   ( tskIDLE_PRIORITY + 4 )   /* 人眼 30fps 满足，晚几帧没事 */
#define TASK_PRIO_STORAGE   ( tskIDLE_PRIORITY + 3 )   /* 扇区擦除 45ms 级，天生慢性子 */
#define TASK_PRIO_MONITOR   ( tskIDLE_PRIORITY + 1 )   /* 高于 idle 不饿死即可 */

/* 栈大小 */
#define TASK_STACK_ACQUIRE  ( 256 )   /* 1KB */
#define TASK_STACK_COMM     ( 512 )   /* 2KB */
#define TASK_STACK_DISPLAY  ( 512 )   /* 2KB */
#define TASK_STACK_STORAGE  ( 256 )   /* 1KB */
#define TASK_STACK_MONITOR  ( 512 )   /* 2KB */
/* 任务句柄 */
extern TaskHandle_t xTaskAcquire;
extern TaskHandle_t xTaskDisplay;
extern TaskHandle_t xTaskComm;
extern TaskHandle_t xTaskStorage;
extern TaskHandle_t xTaskMonitor;
/* 任务创建函数声明 */
void TaskAcquire_Create(void);
void TaskDisplay_Create(void);
void TaskComm_Create(void);
void TaskStorage_Create(void);
void TaskMonitor_Create(void);
#endif /* __APP_TASK_H */
