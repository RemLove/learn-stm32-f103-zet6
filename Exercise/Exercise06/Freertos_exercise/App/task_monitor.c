/* 文件职责：监督心跳 —— 只判"任务死没死"，数据断流单独告警
 *
 * ★ 喂狗判据（整个看门狗设计只有这一条原则，想清楚它其余全顺）
 *   alive 停 = 调度器不转 / 任务死锁 = 不喂狗 → 2s 后 IWDG 复位（这是狗该干的活）
 *   work  停 = 业务断流（ADC 没启、传感器拔了）= 照常喂狗 → 只告警
 *
 * 为什么 work 不能参与喂狗：
 *   断流时复位一次，复位后数据源还是坏的，于是再断流再复位……
 *   板子以 2 秒为周期无限重启，串口狂刷 "[boot] last reset = IWDG !!"，
 *   真正的病因（为什么没数据）被重启循环彻底掩盖，比不加狗还难查。
 *   而"任务卡死"是复位真能救的故障（重新初始化通常就好了），才交给狗。
 */
#include "FreeRTOS.h"
#include "task.h"
#include "app_object.h" //包含全局心跳和事件组
#include "app_task.h"   //包含句柄
#include "app_main.h"   //包含日志
#include <stdio.h>      //包含snprintf(用于把数字转化为字符串)
#include "iwdg.h"
#define MON_PERIOD_MS (1000u)
/* 巡查表数据结构 */
typedef struct
{
    const char *name;           /* 打印用名字 */
    volatile uint32_t *alive;   /* 心跳计数指针：单写多读，monitor 只读 → 喂狗依据 */
    volatile uint32_t *work;    /* 业务计数指针：只用于判断断流，不参与喂狗 */
    TaskHandle_t *handle;       /* 句柄地址：查栈高水位用 */
    uint32_t last_alive;        /* 上轮快照：== 上轮值 = 本轮没转 = 卡死 */
    uint32_t last_work;         /* 上轮快照：== 上轮值 = 本轮没数据 = 断流 */
    uint8_t  nodata;            /* 上轮是否已处于断流态：边沿告警，避免每秒刷屏 */
} mon_item_t;
/* 定义巡查表 */
static mon_item_t s_items[] =
    {
        {"Acquire", &g_aliveAcquire, &g_workAcquire, &xTaskAcquire, 0, 0, 0},
        {"Display", &g_aliveDisplay, &g_workDisplay, &xTaskDisplay, 0, 0, 0},
        {"Storage", &g_aliveStorage, &g_workStorage, &xTaskStorage, 0, 0, 0},
};

#define MON_ITEM_NUM (sizeof(s_items) / sizeof(s_items[0]))

/* 报表缓冲：static 而非栈上。一行最长约 140 字符，192 富余。
 * 注意别按"看着差不多"估长度 —— snprintf 超了会静默截断，
 * 截掉尾巴上的 dD/dS 你还以为是计数器不涨。*/
static char s_buf[192];

static void TaskMonitor(void *pv)
{
    (void)pv;
    TickType_t last_time = xTaskGetTickCount();
    uint32_t round = 0; /* 巡检轮次，报表序号 */
    
    for (;;)
    {
        uint8_t all_alive=1;
        // 稳定1s查看一次（pdMS_TO_TICKS 别省：现在 tick=1000Hz 两者数值恰好相等，
        // 哪天 configTICK_RATE_HZ 改成 100，直接写 1000 就变成 10 秒一轮）
        xTaskDelayUntil(&last_time, pdMS_TO_TICKS(MON_PERIOD_MS));
        // 循环检查 对比心跳是否变化
        for (uint8_t i = 0; i < MON_ITEM_NUM; i++)
        {
            mon_item_t *it = &s_items[i];

            /* ---- 1. 死活：alive 和上轮一样 = 这一秒它一次都没转 ---- */
            if (*it->alive == it->last_alive) // 没变化
            {
                App_Log("[mon] !! FROZEN: %s\r\n", it->name);
                //如果有一项不变化了
                all_alive=0;               /* 有任务卡死 → 本轮不喂狗 */
            }
            else // 变化了 对应逻辑
            {
                // 更新
                it->last_alive = *it->alive;
            }

            /* ---- 2. 业务：work 没动 = 数据断流。只告警，不动 all_alive ---- */
            if (*it->work == it->last_work)
            {
                if (it->nodata == 0)       /* 边沿触发：刚断流时报一次 */
                {
                    App_Log("[mon] -- NO DATA: %s\r\n", it->name);
                    it->nodata = 1;
                }
            }
            else
            {
                if (it->nodata != 0)       /* 恢复时报一次 */
                {
                    App_Log("[mon] ++ DATA BACK: %s\r\n", it->name);
                    it->nodata = 0;
                }
                it->last_work = *it->work;
            }
        }
        if(all_alive==1)
        {
            HAL_IWDG_Refresh(&hiwdg);
        }
        int n = snprintf(s_buf, sizeof(s_buf),
                             "[mon] r=%lu a=%lu/%lu/%lu w=%lu/%lu/%lu hwm=%lu/%lu/%lu idle=%lu heap=%lu dD=%lu dS=%lu\r\n",
                             (unsigned long)round,
                             (unsigned long)g_aliveAcquire,
                             (unsigned long)g_aliveDisplay,
                             (unsigned long)g_aliveStorage,
                             (unsigned long)g_workAcquire,
                             (unsigned long)g_workDisplay,
                             (unsigned long)g_workStorage,
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[0].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[1].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[2].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()),
                             (unsigned long)xPortGetFreeHeapSize(),
                             (unsigned long)g_dropDisplay,
                             (unsigned long)g_dropStorage);
            if (n > 0) /* snprintf 返回"本应写入的长度"，负数=编码错误
                        * n >= sizeof(s_buf) 说明被截断了，说明格式串又变长了 */
            {
                App_Log("%s", s_buf);
            }
        round++;
    }
}

TaskHandle_t xTaskMonitor = NULL;

void TaskMonitor_Create(void)
{
    BaseType_t ok = xTaskCreate(TaskMonitor, "Monitor",
                                TASK_STACK_MONITOR, NULL,
                                TASK_PRIO_MONITOR, &xTaskMonitor);
    configASSERT(ok == pdPASS);
}
