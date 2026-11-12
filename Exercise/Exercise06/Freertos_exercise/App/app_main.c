/* 文件职责：初始化启动 日志打印 */
#include <stdio.h>     /* vprintf */
#include <stdarg.h>    /* va_list：变参转发三件套 */
#include "FreeRTOS.h"
#include "task.h"      /* vTaskStartScheduler / xPortGetFreeHeapSize */
#include "app_object.h"
#include "app_task.h"
#include "app_main.h"
void App_Log( const char *fmt, ... )    /* fmt+"..."：照抄 printf 的签名 */
{
    va_list args;        /* 指向"..."参数区"的游标 */
    va_start( args, fmt );   /* 从 fmt 之后的第一个参数开始数 */

    xSemaphoreTake( xMtxPrintf, portMAX_DELAY );  /* 排队；portMAX_DELAY=
                                                     永远等（拿锁是等"地盘"
                                                     空出来，这是锁的标准等法）*/
    vprintf( fmt, args );    /* vprintf = 吃 va_list 的 printf，
                                把格式串和参数原样转给真 printf */
    xSemaphoreGive( xMtxPrintf );  /* 释放，让下一个人打印 */

    va_end( args );      /* 和 va_start 成对，清理现场 */
}

void Main_Init(void)
{
    //先对象后任务
    App_ObjectsInit();//创建队列信号量啥的
    TaskAcquire_Create();//创建acquire任务
		TaskDisplay_Create();
    vTaskStartScheduler();//启动调取器，cpu开始轮转
}
