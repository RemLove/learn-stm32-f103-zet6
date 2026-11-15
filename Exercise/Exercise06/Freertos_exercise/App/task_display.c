/* 当前文件职责：10Hz 显示任务——滚动波形 + 状态行 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>
#include "app_object.h"    /* qDisplay / xMtxLcd / g_dropXxx / sample_t */
#include "app_task.h"      /* 优先级栈宏 + 句柄定义权 */
#include "lcd.h"           /* DrawFill / DrawLine / WriteAsiciStr，含 LCD_W / LCD_H */

/* 宏定义*/
/* LCD_W / LCD_H 由 lcd.h 提供，这里不再重复定义 —— 同一宏在两处定义，
 * 哪天改屏尺寸只记得改一边，就会出现擦除和画图错位的鬼故事 */
#define LCD_xSTART  10     //x起始位置
#define LCD_ySTART  10     //y起始位置
#define LCD_STEP    30     //步长
#define WAVE_BCOLOR WHITE //波形背光
#define WAVE_FCOLOR RED //波形颜色
#define STR_BCOLOR WHITE  //字的背光(和屏幕背光相同)
#define STR_FCOLOR BLUE   //字的颜色
#define WAVE_YSTART (LCD_H-1)   /* 319：波形区最底一行，mv=0 就画在这 */
#define WAVE_YEND   100         /* 波形区最上一行，mv=3300 画在这 */
#define WAVE_H (WAVE_YSTART-WAVE_YEND)  /* 219 行，是"行数"不是"终点坐标" */
#define MV_MAX 3300

static char s_line[32];//字符串缓冲区 用于把整数转化为字符串

static void TaskDisplay( void *pv )
{
    (void)pv;
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t frame=0;//帧计数 用于到时刷新文字
    uint16_t wave_x=0;//波形笔位x点在0-240来回移动
    uint16_t y_prev=WAVE_YSTART;
    /* 上一帧样本：断流时保留旧值，屏幕不会突然空白，也方便你看数字有没有冻住 */
    sample_t s_last={0};

    for (;;)
    {
        xTaskDelayUntil(&xLastWake,pdMS_TO_TICKS(10));//100帧一秒刷新一次文字

        /* 把队列取空，只留最新一帧。注意这里用 0 超时不是 portMAX_DELAY：
         * 本任务靠"每 10ms 固定节拍"驱动，不该被队列阻塞改变节奏。*/
        sample_t s;
        BaseType_t got=pdFALSE;
        while (xQueueReceive(qDisplay,&s,0)==pdPASS)
        {
            s_last=s;
            got=pdPASS;
        }

        /* ★ frame 无条件递增 —— 文字区每秒照刷。
         * 如果把它塞进 if(got) 里，断流时这行字也一起冻住，
         * 你就没法从屏幕判断"是没数据"还是"整块屏死了"。*/
        frame++;
        if(frame%100==1)
        {
            xSemaphoreTake(xMtxLcd,portMAX_DELAY);
            /* 值区一次性整块擦掉：x=100 起、宽 140、高 90。
             * 原来三条 12px 宽的窄条根本擦不掉 "dropD:1" 变成 "dropD:12" 时
             * 多出来的那个字符 —— 位数一变就留鬼影。*/
            LCD_DrawFill( LCD_xSTART+LCD_STEP*3, LCD_ySTART,
                          LCD_W-(LCD_xSTART+LCD_STEP*3), LCD_STEP*3, STR_BCOLOR );
            //第一行 mv
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART,24,STR_FCOLOR,STR_BCOLOR,"mv:");
            snprintf(s_line,sizeof(s_line),"%4u",s_last.mv);
            LCD_WriteAsiciStr( LCD_xSTART+LCD_STEP*3, LCD_ySTART, 24,
                               STR_FCOLOR, STR_BCOLOR, ( uint8_t * )s_line );
            //第二行 dropD
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,"dropD:");
            snprintf(s_line,sizeof(s_line),"%lu",( unsigned long )g_dropDisplay);
            LCD_WriteAsiciStr(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP,24,STR_FCOLOR,STR_BCOLOR,( uint8_t * )s_line);
            //第三行 dropS
            LCD_WriteAsiciStr(LCD_xSTART,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,"dropS:");
            snprintf(s_line,sizeof(s_line),"%lu",( unsigned long )g_dropStorage);
            LCD_WriteAsiciStr(LCD_xSTART+LCD_STEP*3,LCD_ySTART+LCD_STEP*2,24,STR_FCOLOR,STR_BCOLOR,( uint8_t * )s_line);
            xSemaphoreGive(xMtxLcd);
        }

        /* 波形只在真收到新样本时才推进 —— 没数据就不动笔，笔位不空跑 */
        if (got==pdPASS)
        {
            /* 1.电压 → 屏幕 y 映射
             * 公式：y = 底部 − (电压/满量程)×区高
             *   0mV    → y=319（贴底）
             *   3300mV → y=100（贴顶）
             * 中间量必须 uint32_t：3300×219≈72.3 万，
             * uint16 上限 65535——不改的话映射全乱 */
            uint16_t wave_y = WAVE_YSTART
                       - ( uint16_t )( ( uint32_t )s_last.mv * WAVE_H / MV_MAX );
            //2.x右移 到头返回
            wave_x=(uint16_t)(wave_x+1)%LCD_W;
            //3.一到画图 就拿锁 画完放锁
            xSemaphoreTake( xMtxLcd, portMAX_DELAY );
            /* 4.先擦除 再画
             * 擦除高度必须是 WAVE_H+1=220 行（100~319），不是 219 行。
             * 只擦 219 行覆盖 100~318，而 mv=0 时映射到 y=319 那一行擦不到，
             * 那条像素会一直留在屏幕最底边，拉出一道贯穿全屏的红杠。*/
            LCD_DrawFill( wave_x, WAVE_YEND, 1, WAVE_H+1, WAVE_BCOLOR );
            /* 处理断点 */
            uint16_t y_h=(y_prev<wave_y)?y_prev:wave_y;//用于两点之间连线 防止两点之间跨度太大有断点 从低的开始画
            uint16_t y_gap = ( wave_y > y_prev ) ? ( uint16_t )( wave_y - y_prev )
                                                 : ( uint16_t )( y_prev - wave_y );
            LCD_DrawFill(wave_x, y_h ,1,y_gap+1,WAVE_FCOLOR);
            /* 5.画点：LCD_DrowPoint 是 w×w 的方块，w=3 时从 (x,y) 向右下占 3 列。
             * x=237 时占到 239 刚好贴边；x=238/239 就会占到 240/241，
             * 越过 240 列宽 → 写进下一行开头，屏幕右侧出现花屏。*/
            if (wave_x <= ( LCD_W - 3 ))
            {
                LCD_DrowPoint( wave_x, wave_y, 3, WAVE_FCOLOR );
            }
            //6.放锁
            xSemaphoreGive( xMtxLcd );
            //更新prev
            y_prev = wave_y;

            /* 业务量：真的画了一帧才算 */
            g_workDisplay++;
        }

        /* 心跳：放在所有 if 外面，无条件 +1
         * 断流时这一句仍然在跑 → monitor 知道本任务没死 → 照常喂狗，
         * 只报"NO DATA"告警。复位治不了传感器没插。*/
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
