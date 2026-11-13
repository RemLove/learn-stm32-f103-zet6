/* 当前文件职责：10Hz 显示任务——滚动波形 + 状态行 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "app_object.h"    /* qDisplay / xMtxLcd / g_dropXxx / sample_t */
#include "app_task.h"      /* 优先级栈宏 + 句柄定义权 */
#include "lcd.h"           /* DrawFill / DrawLine / WriteAsiciStr */

/* 宏定义*/
#define LCD_W       240
#define LCD_H       320
#define LCD_xSTART  10     //x起始位置
#define LCD_ySTART  10     //y起始位置
#define LCD_STEP    30     //步长
#define WAVE_BCOLOR WHITE //波形背光
#define WAVE_FCOLOR RED //波形颜色
#define STR_BCOLOR WHITE  //字的背光(和屏幕背光相同)
#define STR_FCOLOR BLUE   //字的颜色
#define NUM_xSTART  40
#define NUM_ySTART  10
#define WAVE_YSTART (320-1)
#define WAVE_YEND   100
#define WAVE_H (WAVE_YSTART-WAVE_YEND)
#define MV_MAX 3300

static char s_line[32];//字符串缓冲区 用于把整数转化为字符串

static void TaskDisplay( void *pv )
{
    (void)pv;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t frame=0;//帧计数 用于到时刷新文字
    uint16_t wave_x=0;//波形笔位x点在0-240来回移动
    uint16_t y_prev=WAVE_YSTART;
    
    for (;;)
    {
        xTaskDelayUntil(&xLastWake,pdMS_TO_TICKS(10));//100帧一秒刷新一次文字
        
        sample_t s={0};
        BaseType_t ret=pdFALSE;
        while (xQueueReceive(qDisplay,&s,0)==pdPASS)
        {
            ret=pdPASS;
        }
        if(ret==pdFAIL)//如果本轮空 跳过
        {
            continue;
        }
        frame++;
        if(frame%100==1)//
        {
            //先拿锁
            xSemaphoreTake(xMtxLcd,portMAX_DELAY);
            //先擦再写 擦一行
            //第一行ms
            LCD_DrawFill(LCD_xSTART,LCD_ySTART,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART,24,STR_FCOLOR,STR_BCOLOR,"mv:");
            /* 把整数转化为字符串 */
            snprintf(s_line,sizeof(s_line),"%4u",s.mv);
            LCD_WriteAsiciStr( LCD_xSTART+LCD_STEP*3, LCD_ySTART, 24,
                               STR_FCOLOR, STR_BCOLOR, ( uint8_t * )s_line );
            //第二行 dropD
            LCD_DrawFill(LCD_xSTART,LCD_ySTART+LCD_STEP,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,"dropD:");
            snprintf(s_line,sizeof(s_line),"%lu",( unsigned long )g_dropDisplay);
            LCD_WriteAsiciStr(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,( uint8_t * )s_line);
            //第三行 dropS
            LCD_DrawFill(LCD_xSTART,LCD_ySTART+LCD_STEP*2,12,24,STR_BCOLOR);
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,"dropS:");
            snprintf(s_line,sizeof(s_line),"%lu",( unsigned long )g_dropStorage);
            LCD_WriteAsiciStr(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,( uint8_t * )s_line);
            xSemaphoreGive(xMtxLcd);
        }
        /* 波形区 */
        /* 1.电压 → 屏幕 y 映射
         * 公式：y = 底部 − (电压/满量程)×区高
         *   0mV    → y=319（贴底）
         *   3300mV → y=99（贴顶）
         * 中间量必须 uint32_t：3300×220≈72.6 万，
         * uint16 上限 65535——不改的话映射全乱 */
        uint16_t wave_y = WAVE_YSTART
                   - ( uint16_t )( ( uint32_t )s.mv * WAVE_H / MV_MAX );
        //2.x右移 到头返回
        wave_x=(uint16_t)(wave_x+1)%LCD_W;
        //3.一到画图 就拿锁 画完放锁
        xSemaphoreTake( xMtxLcd, portMAX_DELAY );
        //4.先擦除 再画
        LCD_DrawFill( wave_x, WAVE_YEND, 1, WAVE_H, WAVE_BCOLOR );
        /* 处理断点 */
        uint16_t y_h=(y_prev<wave_y)?y_prev:wave_y;//用于两点之间连线 防止两点之间跨度太大有断点 从低的开始画
        uint16_t y_gap = ( wave_y > y_prev ) ? ( uint16_t )( wave_y - y_prev )
                                             : ( uint16_t )( y_prev - wave_y );
        LCD_DrawFill(wave_x, y_h ,1,y_gap+1,WAVE_FCOLOR);
        LCD_DrowPoint( wave_x, wave_y, 3, WAVE_FCOLOR );
        //5.放锁
        xSemaphoreGive( xMtxLcd );
        //更新prev
        y_prev = wave_y;
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
