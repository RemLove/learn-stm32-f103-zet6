#include "freertos_demo.h"
#include "key.h"
#include <stdio.h>
#include "event_groups.h"


/* 128 字 = 512 字节。里面要跑 printf（vfprintf 通常吃 200~400 字节栈），
   再叠上函数调用链，configCHECK_FOR_STACK_OVERFLOW=2 很容易一按就进钩子。
   给 256 字 = 1KB，只多花 2KB 堆，F103ZET6 的 17KB heap 扛得住。 */
#define TASK_STACK_DEPTH 256
#define TASK_PRIORITY 1

/* 宏里的位号必须加括号。现在只用在 xEventGroupSetBits / WaitBits 的参数里，
   即使不加也碰巧没事；但哪天写成 EVENT_4 | 3 这种，1<<4|3 会变成 1<<(4|3)。 */
#define EVENT_0 (1u << 0)
#define EVENT_4 (1u << 4)

TaskHandle_t task1_handle = NULL;
TaskHandle_t task2_handle = NULL;
EventGroupHandle_t event_group_handle = NULL;   /* 原 handel 拼错，顺手改 */
TaskHandle_t task_start_handle = NULL;

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
                xEventGroupSetBits(event_group_handle, EVENT_0);
                printf("bit0 set 1\r\n");
                break;

            case KEY_LONG:
                xEventGroupSetBits(event_group_handle, EVENT_4);
                printf("bit4 set 1\r\n");
                break;

            default:
                break;
            }
        }

        /* 按键扫描周期。原来 500ms 太长：Key_Scan 内部消抖是 50ms，
           一次短按"按下 100ms 再松手"很可能整段落在两次扫描之间 —— 直接漏检。
           20ms 一次足够，CPU 几乎不占。 */
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void task2(void *pvParameters)
{
    EventBits_t evt;

    (void)pvParameters;

    /* 任务函数绝不能 return：返回等于把栈顶的现场结构当返回地址弹出去，
       基本就是 HardFault。所以外面必须套 for(;;)。 */
    for (;;)
    {
        /* 参数 3 clearOnExit = pdTRUE：返回时把匹配的位清零，等下一轮重新攒。
           参数 4 waitForAllBits = pdTRUE：bit0 和 bit4 都置位才返回。
              → 只按短按（只置 bit0）不会醒，必须"短按一次 + 长按一次"才醒一次。
              → 想练"任一位就唤醒"，把这里改成 pdFALSE。 */
        evt = xEventGroupWaitBits(event_group_handle,
                                  EVENT_0 | EVENT_4,
                                  pdTRUE,
                                  pdTRUE,
                                  portMAX_DELAY);

        /* 必须接返回值：evt 才是"醒来时哪些位是 1"，
           原来那句只调用不接收，evt 一直是初始化时的 0，打印恒为 0。 */
        printf("wait bits = %#x\r\n", (unsigned int)evt);
    }
}

void task_start(void *pvParameters)
{
    (void)pvParameters;

    /* 这里的临界区其实可以去掉：xTaskCreate 内部自己会进临界区（可嵌套），
       外面再包一层只是把三次 memset 512 字节栈的时间也算进关中断里。 */
    xTaskCreate(task1, "task1", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &task1_handle);
    xTaskCreate(task2, "task2", TASK_STACK_DEPTH, NULL, TASK_PRIORITY, &task2_handle);

    if ((task1_handle == NULL) || (task2_handle == NULL))
    {
        printf("task create failed\r\n");
    }

    vTaskDelete(NULL);       /* 建完就自删，TCB 和栈由 IDLE 任务回收 */
}

void freertos_start(void)
{
    event_group_handle = xEventGroupCreate();
    if (event_group_handle == NULL)
    {
        printf("event group create failed\r\n");
        return;              /* 堆不够，起不来就别往下走 */
    }

    xTaskCreate(task_start, "task_start", TASK_STACK_DEPTH, NULL,
                TASK_PRIORITY, &task_start_handle);

    vTaskStartScheduler();

    /* 堆不够时 vTaskStartScheduler 会直接返回，不写这行代码的话
       main 进空 while(1)，现象是"板子像死了"，分不清是哪个原因。 */
    while (1)
    {
    }
}
