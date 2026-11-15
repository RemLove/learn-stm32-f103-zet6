/* 当前文件职责：数据结构定义和信号量/互斥量/队列/事件标志组 声明 */
#ifndef __APP_OBJECT_H
#define __APP_OBJECT_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/* 队列深度 */
#define QUEUE_LEN_DISPLAY   ( 8 )
#define QUEUE_LEN_STORAGE   ( 32 )
#define QUEUE_LEN_COMM_RX   ( 64 )

/* 阻塞等待超时（ms）
 * 任务里所有"等数据"的地方都必须是这个超时，不能是 portMAX_DELAY。
 * 原因：portMAX_DELAY 一旦等不到数据就永久阻塞，任务再也回不到循环末尾，
 *       心跳自然停 —— monitor 分不清"任务死了"和"数据没来"。
 * 100ms 是折中：远大于一帧数据的到达间隔（10ms），又不至于让心跳太迟钝。*/
#define APP_WAIT_MS         ( 100 )

/* 数据结构定义 */
typedef struct
{
    uint32_t seq;       /* 全局序号：丢帧表现为 seq 缺口 */
    uint32_t tick_ms;   /* 采集时刻的系统 tick */
    uint16_t raw_adc;   /* 0~4095 */
    uint16_t mv;        /* 毫伏值 */
    /*保留字段，用来对齐字节 w25q32一次页写256字节 如果不加reserved 256/12无法写完，现在是256/16刚好整除 */
    uint32_t reserved;
}sample_t;
/* 队列 */
extern QueueHandle_t qDisplay;   /* 采集 -> 显示任务 */
extern QueueHandle_t qStorage;   /* 采集 -> 存储任务 */
extern QueueHandle_t qCommRx;    /* 串口回调 -> 通信任务，元素是 uint8_t */
/* 信号量/互斥量 */
extern SemaphoreHandle_t xSemDmaDone;
extern SemaphoreHandle_t xMtxLcd;     /* 显示任务 和 通信任务(写屏) 共用 */
extern SemaphoreHandle_t xMtxFlash;   /* 存储任务 和 通信任务(ERASE) 共用 */
extern SemaphoreHandle_t xMtxPrintf;  /* 所有想打印的任务共用 */
/* 事件组 */
extern EventGroupHandle_t xEvAlive;

/* 全局变量
 * ★ 两类计数，职责严格分开，混用就出事：
 *   alive —— 每圈循环无条件 +1，只证明"任务在转、没死锁"→ 看门狗只看它
 *   work  —— 真处理了一条业务数据才 +1，反映"数据有没有在流"→ 只告警，不喂狗
 * 为什么不合并：数据源一断（ADC 没启/传感器拔了），alive 照转但 work 停。
 * 若拿 work 当喂狗条件 → 不喂 → 2s 复位 → 复位后照样没数据 → 无限重启循环。
 * 传感器没插是复位治不了的病，报警比自杀有用。*/
extern volatile uint32_t g_aliveAcquire;   /* 心跳：采集任务每圈+1 */
extern volatile uint32_t g_aliveDisplay;   /* 心跳：显示任务每帧+1（阶段3） */
extern volatile uint32_t g_aliveComm;      /* 心跳：通信任务每圈+1（阶段3） */
extern volatile uint32_t g_aliveStorage;   /* 心跳：存储任务每圈+1（阶段3） */
extern volatile uint32_t g_workAcquire;    /* 业务：真正搬走一次 DMA 样本 +1 */
extern volatile uint32_t g_workDisplay;    /* 业务：真正画了一帧波形 +1 */
extern volatile uint32_t g_workComm;       /* 业务：真正解析一条命令 +1（阶段3） */
extern volatile uint32_t g_workStorage;    /* 业务：真正写入一条记录 +1 */
extern volatile uint32_t g_dropDisplay;    /* qDisplay 满被丢弃的帧数 */
extern volatile uint32_t g_dropStorage;    /* qStorage 满被丢弃的帧数 */

void App_ObjectsInit(void);

#endif /* __APP_OBJECT_H */

