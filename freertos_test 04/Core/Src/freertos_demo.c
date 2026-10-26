#include "freertos_demo.h"
#include "key.h"
#include "gpio.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
/*
 * 练习四 计数信号量：KEY0 短按 Give 信号量+1，长按 Take 信号量-1（上限10）
 */

#define TASK_STACK_DEPTH 128//栈深度128字   128*4字节
#define TASK_PRIORITY 1//任务优先级

//任务句柄
TaskHandle_t task1_handle=NULL;
TaskHandle_t task2_handle=NULL;
QueueHandle_t sem_handle=NULL;
TaskHandle_t task_start_handle=NULL;

UBaseType_t count=0;

void task1(void *pvParameters)
{
    Key_Event_t evt;              /* 声明必须放函数开头（armcc 默认 C90 规矩） */
    for (;;)
    {
        Key_Scan();

        evt = key_evt;            /* 1. 取走：拷贝到局部变量 */
        if (evt != KEY_NONE)
        {
            key_evt = KEY_NONE;   /* 2. 立刻清零——只此一处，任何事件都漏不掉 */
            switch (evt)          /* 3. 处理的是局部副本 evt，不是 key_evt */
            {
            case KEY_SHORT://短按一次释放一次信号量 信号量+1 长按一次 获取一次信号量 信号量-1
                xSemaphoreGive(sem_handle);
                count = uxSemaphoreGetCount(sem_handle);
                printf("可用信号量为:%d\r\n",count);
                break;
            case KEY_LONG:
                xSemaphoreTake(sem_handle,0);
                count = uxSemaphoreGetCount(sem_handle);
                printf("可用信号量为:%d\r\n",count);
                break;
            default:
                break;            /* KEY_UP/KEY_DOWN 落到 default，直接忽略 */
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

//每一秒获取当前可用信号量
void task2(void *pvParameters)
{
    for (;;)
    {
        count = uxSemaphoreGetCount(sem_handle);
        printf("可用信号量为:%d\r\n",count);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}



//任务三:这个任务用来启动两个任务,用完就删
void task_start(void*pvParameters)
{

    //进入临界区
    taskENTER_CRITICAL();
    xTaskCreate(task1,
                        "task1",
                        TASK_STACK_DEPTH,
                        NULL,
                        TASK_PRIORITY,
                        &task1_handle
                        );
     xTaskCreate(task2,
                        "task2",
                        TASK_STACK_DEPTH,
                        NULL,
                        TASK_PRIORITY,
                        &task2_handle
                        );
                   
   taskEXIT_CRITICAL();
   //删除任务自己
   vTaskDelete(NULL);

}

void freertos_start(void)
{
    //创建信号量
    sem_handle = xSemaphoreCreateCounting(10,0);
    if(sem_handle==NULL)
    {
        printf("创建信号量失败\r\n");
    }
    else 
    {
        count = uxSemaphoreGetCount(sem_handle);
        printf("可用信号量为:%d\r\n",count);

        printf("创建信号量成功\r\n");
    }

    xTaskCreate(task_start,
                            "task_start",
                             TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY,
                            &task_start_handle
                            );
    //2.启动调度器
    vTaskStartScheduler();
}



