#include "freertos_demo.h"
#include "key.h"

#include <stdio.h>
#include "task.h"
#include "timers.h"

/* 栈深度单位是"字"(StackType_t, 4 字节), 不是字节。
   128 字 = 512B, 里面跑 printf(vfprintf 吃 200~400B) 太紧,
   configCHECK_FOR_STACK_OVERFLOW=2 开着, 容易一按就进钩子。
   给 256 字 = 1KB。 */
#define TASK_STACK_DEPTH     256
#define MON_STACK_DEPTH      256

/* 优先级分层。configMAX_PRIORITIES = 5, 合法值 0~4, 数字越大优先级越高:
     0  IDLE 空闲任务(系统自带)
     1  task1 按键扫描 / task3 监控 —— 有空就跑, 慢一点无所谓
     2  task2 通知响应            —— 定时器一响就要处理
     3  定时器守护任务(configTIMER_TASK_PRIORITY) —— 保证定时器准时
   守护任务必须比调用 xTimerStart 的任务优先级高,
   否则命令排进队列却没人及时处理。 */
#define PRIO_SCAN            1
#define PRIO_MONITOR         1
#define PRIO_NOTIFY          2

/* 定时器 ID。创建时塞进去, 回调里用 pvTimerGetTimerID 取出来区分是哪个定时器;
   同时直接当"任务通知值"的位掩码用 —— 一个值干两件事。 */
#define NOTIFY_BIT_1         0x01u
#define NOTIFY_BIT_2         0x02u

#define TIMER_ID_1           ((void *)NOTIFY_BIT_1)
#define TIMER_ID_2           ((void *)NOTIFY_BIT_2)


TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;
TaskHandle_t task3_handle = NULL;

TimerHandle_t timer1_handle = NULL;
TimerHandle_t timer2_handle = NULL;

TaskHandle_t task_start_handle = NULL;

/* 定时器回调里累加的计数。回调跑在"定时器守护任务"上下文,
   和读它的 task3 是不同任务, 加 volatile 保证每次都从内存读。 */
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
                /* 第 2 个参数是"命令入队等待时间", 不是定时器周期 */
                xTimerStart(timer1_handle, pdMS_TO_TICKS(100));
                printf("开启 timer1（单次 500ms）\r\n");
                break;

            case KEY_LONG:
                xTimerStart(timer2_handle, pdMS_TO_TICKS(100));
                printf("开启 timer2（周期 500ms）\r\n");
                break;

            default:
                break;
            }
        }

        /* 按键扫描周期 20ms: 比 Key_Scan 内部 50ms 消抖更快, 否则漏检 */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task2(void *pvParameters)
{
    uint32_t bits;

    (void)pvParameters;

    for (;;)
    {
        /* 纯事件驱动: 不设超时。
           任务挂在通知等待链表上时不参与调度, 一个 CPU 周期都不占;
           通知一到立刻被唤醒(优先级 2, 比扫描任务高, 响应快)。
             参数 1 = 进等待前清哪些位 (0 = 不动)
             参数 2 = 退出等待时清哪些位 (全清, 每次醒来重新攒)
             参数 3 = 收到的通知值 */
        xTaskNotifyWait(0x00, 0xFFFFFFFFUL, &bits, portMAX_DELAY);

        if ((bits & NOTIFY_BIT_1) != 0)
        {
            printf("timer1 到点（单次）  累计=%lu\r\n", (unsigned long)s_timer1_cnt);
        }

        if ((bits & NOTIFY_BIT_2) != 0)
        {
            printf("timer2 到点（周期）  累计=%lu\r\n", (unsigned long)s_timer2_cnt);
        }
    }
}

void task3(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        /* uxTaskGetStackHighWaterMark 返回"栈最多用到离栈底还剩多少字",
           不是字节, 也不是已用量。数字小 = 曾经吃得很深, 接近 0 就该加栈。
           传 NULL 表示查自己。
           xPortGetFreeHeapSize 是当前剩余堆; xPortGetMinimumEverFreeHeapSize
           是开机以来的最低值 —— 后者才是真实峰值占用, 前者会被释放动作骗到。 */
        printf("[mon] heap_free=%lu heap_min=%lu  stack_hw(字) t1=%lu t2=%lu t3=%lu\r\n",
               (unsigned long)xPortGetFreeHeapSize(),
               (unsigned long)xPortGetMinimumEverFreeHeapSize(),
               (unsigned long)uxTaskGetStackHighWaterMark(task1_handle),
               (unsigned long)uxTaskGetStackHighWaterMark(task2_handle),
               (unsigned long)uxTaskGetStackHighWaterMark(NULL));

        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void task_start(void *pvParameters)
{
    (void)pvParameters;

    xTaskCreate(task1, "task1", TASK_STACK_DEPTH, NULL, PRIO_SCAN,    &task1_handle);
    xTaskCreate(task2, "task2", TASK_STACK_DEPTH, NULL, PRIO_NOTIFY,  &task2_handle);
    xTaskCreate(task3, "task3", MON_STACK_DEPTH,  NULL, PRIO_MONITOR, &task3_handle);

    if ((task1_handle == NULL) || (task2_handle == NULL) || (task3_handle == NULL))
    {
        printf("task create failed\r\n");
    }

    vTaskDelete(NULL);
}

/* 两个定时器共用一个回调, 靠 pvTimerGetTimerID 区分。
   回调由"定时器守护任务"调用, 不是中断上下文, 所以可以用 xTaskNotify
   这类非 FromISR 版本(但不能用会阻塞的 API, 比如 vTaskDelay)。 */
static void timer_action(TimerHandle_t xTimer)
{
    uint32_t id = (uint32_t)pvTimerGetTimerID(xTimer);

    if (id == NOTIFY_BIT_1)
    {
        s_timer1_cnt++;          /* 单次定时器: 到点只进来一次 */
    }
    else
    {
        s_timer2_cnt++;          /* 周期定时器: 每 500ms 进来一次 */
    }

    if (task2_handle != NULL)
    {
        /* eSetBits: 把 id 的位 OR 进 task2 的通知值。
           用 xTaskNotifyGive(eIncrement) 只能表达"有几个通知",
           传不了"是哪个定时器"; eSetBits 可以把信息一起带过去。 */
        xTaskNotify(task2_handle, id, eSetBits);
    }
}

void freertos_start(void)
{
    timer1_handle = xTimerCreate("timer1",
                                 pdMS_TO_TICKS(500),
                                 pdFALSE,          /* pdFALSE = 单次, 触发一次就停 */
                                 TIMER_ID_1,
                                 timer_action);

    timer2_handle = xTimerCreate("timer2",
                                 pdMS_TO_TICKS(500),
                                 pdTRUE,           /* pdTRUE = 周期, 自动重装 */
                                 TIMER_ID_2,
                                 timer_action);

    if ((timer1_handle == NULL) || (timer2_handle == NULL))
    {
        printf("timer create failed（堆不够或 configUSE_TIMERS 没开）\r\n");
        return;
    }

    xTaskCreate(task_start, "task_start", TASK_STACK_DEPTH, NULL,
                PRIO_SCAN, &task_start_handle);

    vTaskStartScheduler();

    /* 堆不够时 vTaskStartScheduler 直接返回, 不写这行会表现得像死机 */
    while (1)
    {
    }
}
