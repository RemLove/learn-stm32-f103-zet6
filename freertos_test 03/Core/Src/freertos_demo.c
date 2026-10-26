#include "freertos_demo.h"
#include "key.h"
#include "gpio.h"
#include <stdio.h>
#include "queue.h"
/*
 * ????? ??????? ???��???????��? freertos????????5-15????????????��? ????��?4 ???6
 */

#define TASK_STACK_DEPTH 128//????128??   128*4???
#define TASK_PRIORITY 1//?????????

//??????
TaskHandle_t task1_handle=NULL;
TaskHandle_t task2_handle=NULL;
TaskHandle_t task3_handle=NULL;
QueueHandle_t queue1_handle=NULL;
QueueHandle_t queue2_handle=NULL;
TaskHandle_t task_start_handle=NULL;

//?????
void task1(void *pvParameters)
{
    char*p="Satella love!!!";
    Key_Event_t evt;              /* ?????????????????armcc ??? C90 ???? */
    BaseType_t res;
    for (;;)
    {
        Key_Scan();

        evt = key_evt;            /* 1. ????????????????? */
        if (evt != KEY_NONE)
        {
            key_evt = KEY_NONE;   /* 2. ???????????????????��??????????? */

           // printf("evt=%d\r\n", evt);

            switch (evt)          /* 3. ?????????????? evt?????? key_evt */
            {
            case KEY_SHORT:
                res = xQueueSend(queue1_handle,&evt,portMAX_DELAY);
                if(res==pdPASS)
                {
                    printf("发送queue1成功\r\n");
                }
                else
                {
                    printf("发送queue1失败\r\n");
                }
                break;
            case KEY_LONG:
                res = xQueueSend(queue2_handle,&p,portMAX_DELAY);
                if(res==pdPASS)
                {
                    printf("发送queue2成功\r\n");
                }
                else
                {
                    printf("发送queue2失败\r\n");
                }
                break;
            default:
                break;            /* KEY_UP/KEY_DOWN ????? evt ????? */
            }
        }
    }
}

uint8_t ret[10];
void task2(void *pvParameters)
{
    BaseType_t res;
    res = xQueueReceive(queue1_handle,&ret,portMAX_DELAY);
    if(res==pdPASS)
    {
        printf("queue2接收成功\r\n");
    }
    else 
    {
        printf("queue2接收失败\r\n");
    }
}

void task3(void *pvParameters)
{
    char*re_buff;
    BaseType_t res;
    res = xQueueReceive(queue1_handle,&re_buff,portMAX_DELAY);
    if(res==pdPASS)
    {
        printf("%s\r\n",re_buff);
        printf("queue1接收成功\r\n");
    }
    else 
    {
        printf("queue1接收失败\r\n");
    }
}


//??????:???????????????????????,??????
void task_start(void*pvParameters)
{

    //?????????
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
    xTaskCreate(task3,
                        "task3",
                        TASK_STACK_DEPTH,
                        NULL,
                        TASK_PRIORITY,
                        &task3_handle
                        );                    
   taskEXIT_CRITICAL();
    //??????????
   vTaskDelete(NULL);

}

void freertos_start(void)
{
    //创建队列
    queue1_handle=xQueueCreate(2,sizeof(uint8_t));
    if(queue1_handle!=NULL)
    {
        printf("queue1创建成功\r\n");
    }
    else
    {
        printf("创建queue1失败\r\n");
    }

     queue2_handle=xQueueCreate(2,sizeof(char*));
    if(queue2_handle!=NULL)
    {
        printf("queue2创建成功\r\n");
    }
    else
    {
        printf("queue2创建失败\r\n");
    }
    xTaskCreate(task_start,
                            "task_start",
                             TASK_STACK_DEPTH,
                            NULL,
                            TASK_PRIORITY,
                            &task_start_handle
                            );
    //2.??????????
    vTaskStartScheduler();
}



