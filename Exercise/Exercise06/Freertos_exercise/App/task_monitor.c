/* 文件职责监督心跳 全部正常 喂狗 */
#include "FreeRTOS.h"
#include "task.h"
#include "app_object.h" //包含全局心跳和事件组
#include "app_task.h"   //包含句柄
#include "app_main.h"   //包含日志
#include <stdio.h>      //包含snprintf(用于把数字转化为字符串)

#define MON_PERIOD_MS (1000u)
/* 巡查表数据结构 */
typedef struct
{
    const char *name;         /* 打印用名字 */
    volatile uint32_t *alive; /* 心跳计数指针：单写多读，monitor 只读 */
    TaskHandle_t *handle;     /* 句柄地址：查栈高水位用 */
    uint32_t last;            /* 上秒快照：== 上秒值 = 本秒没跑 */
} mon_item_t;
/* 定义巡查表 */
static mon_item_t s_items[] =
    {
        {"Acquire", &g_aliveAcquire, &xTaskAcquire, 0},
        {"Display", &g_aliveDisplay, &xTaskDisplay, 0},
        {"Storage", &g_aliveStorage, &xTaskStorage, 0},
};

#define MON_ITEM_NUM (sizeof(s_items) / sizeof(s_items[0]))

/* 报表缓冲：static 而非栈上，一行最长约 95 字符，128 富余 */
static char s_buf[128];

static void TaskMonitor(void *pv)
{
    (void)pv;
    TickType_t last_time = xTaskGetTickCount();
    uint32_t round = 0; /* 巡检轮次，报表序号 */
    for (;;)
    {

        // 稳定1s查看一次
        xTaskDelayUntil(&last_time, MON_PERIOD_MS);
        // 循环检查 对比心跳是否变化 如果变化正常 不变化错误
        for (uint8_t i = 0; i < MON_ITEM_NUM; i++)
        {
            if (*s_items[i].alive == s_items[i].last) // 没变化
            {
                // 实现对应逻辑
                App_Log("[mon] !! FROZEN: %s\r\n", s_items[i].name);
            }
            else // 变化了 对应逻辑
            {
                // 更新
                s_items[i].last = *s_items[i].alive;
                // 喂狗
            }
        }
        int n = snprintf(s_buf, sizeof(s_buf),
                             "[mon] r=%lu acq=%lu dsp=%lu sto=%lu hwm acq=%lu dsp=%lu sto=%lu idle=%lu heap=%lu dD=%lu dS=%lu\r\n",
                             (unsigned long)round,
                             (unsigned long)g_aliveAcquire,
                             (unsigned long)g_aliveDisplay,
                             (unsigned long)g_aliveStorage,
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[0].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[1].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(*s_items[2].handle),
                             (unsigned long)uxTaskGetStackHighWaterMark(xTaskGetIdleTaskHandle()),
                             (unsigned long)xPortGetFreeHeapSize(),
                             (unsigned long)g_dropDisplay,
                             (unsigned long)g_dropStorage);
            if (n > 0) /* snprintf 返回"本应写入的长度"，负数=编码错误 */
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
