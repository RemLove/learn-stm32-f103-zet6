#include "freertos_demo.h"
#include "key.h"

#include <stdio.h>
#include "task.h"
#include "timers.h"

/* 128 字 = 512 字节，里面跑 printf（vfprintf 要 200~400 字节栈）太紧，
   configCHECK_FOR_STACK_OVERFLOW=2 已经开着，容易一按就进钩子死循环。
   给 256 字 = 1KB。 */
#define TASK_STACK_DEPTH 256
#define TASK_PRIORITY 1


TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;

TimerHandle_t timer1_handle = NULL;
TimerHandle_t timer2_handle = NULL;

TaskHandle_t task_start_handle = NULL;

/* 定时器回调里累加的计数。
   回调跑在"定时器守护任务"的上下文里，和别处的代码是不同任务，
   加 volatile 让编译器每次都从内存读，不会被优化成寄存器缓存。 */
static volatile uint32_t s_timer1_cnt = 0;
static volatile uint32_t s_timer2_cnt = 0;


void task1(void *pvParameters)
{
    Key_Event_t evt;

    (void)pvParameters;

    for (;;)
    {
        Key_Scan();
        evt = key_evt;
        if (evt != KEY_NONE)
        {
            key_evt = KEY_NONE;
            switch (evt)
            {
            case KEY_SHORT:
                /* 第 2 个参数是"命令入队等待时间"，不是定时器周期！
                   队列满时最多等 100ms 就放弃，不要用 portMAX_DELAY 死等。 */
                xTimerStart(timer1_handle, pdMS_TO_TICKS(100));
                printf("开启 timer1（单次 500ms，只跳一次）\r\n");
                break;

            case KEY_LONG:
                xTimerStart(timer2_handle, pdMS_TO_TICKS(100));
                printf("开启 timer2（周期 500ms，一直跳）\r\n");
                break;

            default:
                break;
            }
        }

        /* 按键扫描周期 50ms -> 20ms，比 Key_Scan 内部 50ms 消抖更快，避免漏检 */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task2(void *pvParameters)
{
    uint32_t got;
    uint32_t notify_total = 0;

    (void)pvParameters;

    for (;;)
    {
        /* ulTaskNotifyTake(pdTRUE, ...) 是"任务通知"里最好用的一个：
             pdTRUE      = 取完把通知值清零（当二值/计数信号量用）
             第 2 个参数 = 最多等多久
           返回 0 表示等超时了，非 0 表示拿到了几个通知。
           这里用 500ms 超时：即使没人发通知，也能周期性把计数器打出来，
           方便你确认"定时器到底跑没跑"。 */
        got = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500));
        if (got != 0)
        {
            notify_total += got;
        }

        printf("notify=%lu  timer1=%lu  timer2=%lu\r\n",
               (unsigned long)notify_total,
               (unsigned long)s_timer1_cnt,
               (unsigned long)s_timer2_cnt);
    }
}

void task_start(void *pvParameters)
{
    (void)pvParameters;

    xTaskCreate(task1, "task1", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &task1_handle);
    xTaskCreate(task2, "task2", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &task2_handle);

    if ((task1_handle == NULL) || (task2_handle == NULL))
    {
        printf("task create failed\r\n");
    }

    vTaskDelete(NULL);
}

/* 定时器回调。注意：它由"定时器守护任务"调用，不是中断上下文，
   所以可以用 xTaskNotifyGive 这类非 FromISR 版本的 API（不能用会阻塞的 API）。 */
static void timer1_action(TimerHandle_t xTimer)
{
    (void)xTimer;
    s_timer1_cnt++;          /* 单次定时器：到点只进来一次 */
}

static void timer2_action(TimerHandle_t xTimer)
{
    (void)xTimer;
    s_timer2_cnt++;          /* 周期定时器：每 500ms 进来一次 */

    /* 关键：给 task2 发通知，等价于"给一个二值信号量"。
       原来没有这一句，task2 就永远阻塞在 portMAX_DELAY 上一动不动。 */
    if (task2_handle != NULL)
    {
        xTaskNotifyGive(task2_handle);
    }
}

void freertos_start(void)
{
    /* 第 4 个参数 pvTimerID 是给你自己用的标签，回调里可用
       pvTimerGetTimerID(xTimer) 取出来区分是哪个定时器。
       原来两个都传字符串 "1"，编译器会把相同字面量合到同一地址，
       两个 ID 就完全一样了，失去区分意义 —— 这里改成不同的整数。 */
    timer1_handle = xTimerCreate("timer1",
                                 pdMS_TO_TICKS(500),
                                 pdFALSE,          /* pdFALSE = 单次，触发一次就停 */
                                 (void *)1,
                                 timer1_action);

    timer2_handle = xTimerCreate("timer2",
                                 pdMS_TO_TICKS(500),
                                 pdTRUE,           /* pdTRUE = 周期，自动重装 */
                                 (void *)2,
                                 timer2_action);

    if ((timer1_handle == NULL) || (timer2_handle == NULL))
    {
        printf("timer create failed（堆不够或 configUSE_TIMERS 没开）\r\n");
        return;
    }

    xTaskCreate(task_start, "task_start", TASK_STACK_DEPTH, NULL,
                TASK_PRIORITY, &task_start_handle);

    vTaskStartScheduler();

    /* 堆不够时 vTaskStartScheduler 直接返回，不写这行会表现得像死机 */
    while (1)
    {
    }
}
