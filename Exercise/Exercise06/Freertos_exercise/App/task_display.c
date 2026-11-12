/* 当前文件职责：30Hz 显示任务——滚动波形 + 状态行 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "app_object.h"    /* qDisplay / xMtxLcd / g_dropXxx / sample_t */
#include "app_task.h"      /* 优先级栈宏 + 句柄定义权 */
#include "lcd.h"           /* DrawFill / DrawLine / WriteAsiciStr */

/* 宏定义*/
#define LCD_W      240
#define LCD_H      320
#define LCD_xSTART 10     //x起始位置
#define LCD_ySTART 10     //y起始位置
#define LCD_STEP   30     //步长
#define WAVE_BCOLOR WHITE //波形背光
#define WAVE_FCOLOR RED //波形颜色
#define STR_BCOLOR WHITE  //字的背光(和屏幕背光相同)
#define STR_FCOLOR BLUE   //字的颜色
#define NUM_xSTART 40
#define NUM_ySTART 10



static void TaskDisplay( void *pv )
{
    (void)pv;
    static uint16_t s_line={0};//队列接收缓冲区 
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t frame=0;//帧计数 用于到时刷新文字
    for (;;)
    {
        xTaskDelayUntil(&xLastWake,pdMS_TO_TICKS(30));//30帧一秒刷新一次文字
        
        sample_t s={0};
        BaseType_t ret=pdFALSE;
        if(xQueueReceive(qDisplay,&s,0)==pdTRUE) //如果接收到数据
        {
            frame++;
        }
        else//如果没有接收到数据跳过本轮
        {
            continue;
        }
        if(frame%30==0)//30帧一秒 到30帧了要刷新了
        {
            //先拿锁
            xSemaphoreTake(xMtxLcd,portMAX_DELAY);
            //先擦再写 擦一行
            //第一行ms
            LCD_DrawFill(LCD_xSTART,LCD_ySTART,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART,24,STR_FCOLOR,STR_BCOLOR,"mv:");
            LCD_WriteAsiciChar(LCD_xSTART+LCD_STEP*3,LCD_ySTART,24,STR_FCOLOR,STR_BCOLOR,(uint32_t)s.mv);
            //第二行 dropD
            LCD_DrawFill(LCD_xSTART,LCD_ySTART+LCD_STEP,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,"dropD:");
            LCD_WriteAsiciChar(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,g_dropDisplay);
            //第三行 dropS
            LCD_DrawFill(LCD_xSTART,LCD_ySTART+LCD_STEP*2,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,"dropS:");
            LCD_WriteAsiciChar(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,g_dropStorage);
        }
        //心跳+1
        g_aliveDisplay++;
    }
}

/* 句柄 */
TaskHandle_t xTaskDisplay = NULL;

void TaskDisplay_Create( void )
{
    BaseType_t ok = xTaskCreate( TaskDisplay,        /* 任务函数 */
                                 "Display",          /* 名字：调试器和溢出钩子显示用 */
                                 TASK_STACK_DISPLAY, /* 栈 512 word = 2KB（snprintf+锁）*/
                                 NULL,               /* 传给 pv 的参数，不用 */
                                 TASK_PRIO_DISPLAY,  /* 优先级 4 */
                                 &xTaskDisplay );    /* 回填句柄 */
    configASSERT( ok == pdPASS );   /* 失败=堆不够，死在明处别带病运行 */
}
