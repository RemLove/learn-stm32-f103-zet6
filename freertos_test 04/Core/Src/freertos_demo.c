#include "freertos_demo.h"
#include "key.h"
#include <stdio.h>
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"


#define TASK_STACK_DEPTH 128
#define TASK_PRIORITY 1

#define EVENT_0 1<<0
#define EVENT_4 1<<4

TaskHandle_t task1_handle=NULL;
TaskHandle_t task2_handle=NULL;
EventGroupHandle_t event_group_handel=NULL;
TaskHandle_t task_start_handle=NULL;

UBaseType_t count=0;

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
                xEventGroupSetBits(event_group_handel,EVENT_0);\
                printf("bit0 set 1\r\n");
                break;
            case KEY_LONG:
                xEventGroupSetBits(event_group_handel,EVENT_4);
                printf("bit4 set 1\r\n");
                break;
            default:
                break;            
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void task2(void *pvParameters)
{
    EventBits_t evt=0;
    xEventGroupWaitBits(event_group_handel,EVENT_0|EVENT_4,pdTRUE,pdTRUE,portMAX_DELAY);
    printf("%#x\r\n",evt);
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

void freertos_start(void)
{
   //创建事件标志组
   event_group_handel = xEventGroupCreate();
    xTaskCreate(task_start,
                            "task_start",
                             TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY,
                            &task_start_handle
                            );
    //2.����������
    vTaskStartScheduler();
}



