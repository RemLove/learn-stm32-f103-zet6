#include "freertos_demo.h"
#include "key.h"

#include <stdio.h>
#include "task.h"
#include "timers.h"

#define TASK_STACK_DEPTH 128
#define TASK_PRIORITY 1


TaskHandle_t task1_handle=NULL;
TaskHandle_t task2_handle=NULL;

TimerHandle_t timer1_handle=NULL;
TimerHandle_t timer2_handle=NULL;

TaskHandle_t task_start_handle=NULL;


void task1(void *pvParameters)
{
    Key_Event_t evt;          
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
                xTimerStart(timer1_handle,portMAX_DELAY);
               printf("开启timer1\r\n");
                break;
            case KEY_LONG:
            xTimerStart(timer2_handle,portMAX_DELAY);
                printf("开启timer2\r\n");
                break;
            default:
                break;         
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task2(void *pvParameters)
{
    uint32_t j=0;
    for (;;)
    {
        printf("task2正在运行\r\n");
        xTaskNotifyWait(0x00000000,0xffffffff,&j,portMAX_DELAY);
        printf("%d\r\n",j);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void task_start(void*pvParameters)
{


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

   vTaskDelete(NULL);

}
uint32_t i=0;
void (timer1_action)( TimerHandle_t xTimer )
{
    i++;
}
uint32_t j=0;
void (timer2_action)( TimerHandle_t xTimer )
{
    j++;
}
void freertos_start(void)
{
    //创建软件定时器
    timer1_handle = xTimerCreate( //没有重装 只溢出一次
                            "timer1",
                            pdMS_TO_TICKS(500),
                            pdFALSE,
                            "1",
                            timer1_action
                            );
    timer2_handle = xTimerCreate(//500ms溢出一次
                            "timer2",
                            pdMS_TO_TICKS(500),
                            pdTRUE,
                            "1",
                            timer2_action
                            );
    xTaskCreate(task_start,
                            "task_start",
                             TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY,
                            &task_start_handle
                            );

    vTaskStartScheduler();
}



