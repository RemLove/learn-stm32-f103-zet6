#include "freertos_demo.h"
#include "key.h"
#include "gpio.h"
#include "usart.h"
#include <stdio.h>

/*
 * 练习一 三个任务 + 三个按键控制
 *   led0_task : 间隔 200ms 闪烁
 *   led1_task : 间隔 500ms 闪烁
 *   key0_task : 按键处理
 *               KEY0 短按 -> 挂起 led0_task
 *               KEY0 长按 -> 恢复 led0_task
 *               KEY_UP    -> 挂起总调度器
 *               KEY1      -> 恢复总调度器
 */

#define TASK_STACK_DEPTH 128//栈深度128字   128*4字节
#define TASK_PRIORITY 1//任务优先级

//任务句柄
TaskHandle_t led0_handle=NULL;
TaskHandle_t led1_handle=NULL;
TaskHandle_t key0_handle=NULL;
TaskHandle_t task_start_handle=NULL;

//任务一
void led0_task(void*pvParameters)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

}

//任务二
void led1_task(void*pvParameters)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

//任务三:按键处理
//按键消抖+短按/长按判断交给 Interface/KEY/key.c 的状态机,
//这里只负责把 key_evt 事件翻译成任务控制动作。
void key0_task(void*pvParameters)
{
    Key_Event_t evt;
    uint8_t led0_suspended=0;    //led0任务是否已挂起
    uint8_t sched_suspended=0;   //调度器是否已挂起

    for (;;)
    {
        //扫描按键,内部有50ms消抖和长按判定
        Key_Scan();

        //取事件并清标志(事件是一次性的,取走就要清)
        evt=key_evt;
        key_evt=KEY_NONE;

        switch(evt)
        {
            case KEY_SHORT:            //KEY0短按:挂起led0任务
                if(led0_suspended==0 && led0_handle!=NULL)
                {
                    vTaskSuspend(led0_handle);
                    led0_suspended=1;
                    printf("挂起task1\r\n");
                }
                break;

            case KEY_LONG:             //KEY0长按:恢复led0任务
                if(led0_suspended==1 && led0_handle!=NULL)
                {
                    vTaskResume(led0_handle);
                    led0_suspended=0;
                    printf("恢复task1\r\n");
                }
                break;

            case KEY_UP:               //KEY_UP:挂起总调度器
                if(sched_suspended==0)
                {
                    vTaskSuspendAll();
                    sched_suspended=1;
                    printf("挂起总调度器\r\n");
                    //注意:调度器挂起后不能再 vTaskDelay(会死锁)，
                    //     因为挂起调度器后当前任务仍能跑，但延时队列不再被处理。
                    //     恢复靠 KEY1,但那时 key0_task 自己已经停住了,
                    //     所以这里用轮询等待 KEY1 物理引脚电平变化。
                    //     (中断仍能响应,所以也可以用 EXTI 来做,见下方说明)
                    while(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)!=GPIO_PIN_RESET)
                    {
                        //等待 KEY1 被按下(低电平)
                    }
                    xTaskResumeAll();
                    sched_suspended=0;
                    printf("恢复总调度器\r\n");
                }
                break;

            case KEY_DOWN:             //KEY1:恢复总调度器
                if(sched_suspended==1)
                {
                    xTaskResumeAll();
                    sched_suspended=0;
                    printf("恢复总调度器\r\n");
                }
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//堆分配失败钩子:堆不够时停在这里,配合调试器能立刻定位
//(configUSE_MALLOC_FAILED_HOOK=1 时 FreeRTOS 会调用它)
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for( ; ; )
    {
    }
}

//栈溢出钩子:任务栈用完了,停在这里能立刻知道是哪个任务
//(configCHECK_FOR_STACK_OVERFLOW=2 时 FreeRTOS 会调用它)
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for( ; ; )
    {
    }
}

//任务四:这个任务用来启动三个任务,用完就删
void task_start(void*pvParameters)
{

    //进入临界区
    taskENTER_CRITICAL();
    xTaskCreate(led0_task,
                            "led0_task",
                            TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY,
                            &led0_handle
                            );

    xTaskCreate(led1_task,
                            "led1_task",
                             TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY+1,
                            &led1_handle
                            );

    xTaskCreate(key0_task,
                        "key0_task",
                        TASK_STACK_DEPTH,
                        NULL,
                        TASK_PRIORITY+2,
                        &key0_handle
                        );
     //退出临界区
   taskEXIT_CRITICAL();
    //删除任务自己
   vTaskDelete(NULL);

}

void freertos_start(void)
{
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
