
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_demo.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
#include "queue.h"
//启动文件的配置
#define TASK_STACK 128
#define TASK_PRIORITY 1
TaskHandle_t start_task_handle;
void task_start(void*pv);

//任务1的配置
#define TASK1_STACK 128
#define TASK1_PRIORITY 2
TaskHandle_t task1_handle;
void task1(void*pv);

//任务2的配置
#define TASK2_STACK 128
#define TASK2_PRIORITY 3
TaskHandle_t task2_handle;
void task2(void*pv);

QueueHandle_t queue1;
QueueHandle_t big_queue;


void FreeRtos_Start(void)
{

    queue1=xQueueCreate(2,sizeof(uint8_t));
    if(queue1!=NULL)
    {
        printf("creat queue1 success!\r\n");
    }
    else 
    {
        printf("creat queue1 fail!\r\n");
    }
    big_queue=xQueueCreate(1,sizeof(char*));
    if(queue1!=NULL)
    {
        printf("creat big_queue success!\r\n");
    }
    else 
    {
        printf("creat big_queue fail!\r\n");
    }

    //1.创建一个启动函数
     xTaskCreate( (TaskFunction_t) task_start,
                             (char *)  "task_start", 
                             (configSTACK_DEPTH_TYPE) TASK_STACK,
                            (void *) NULL,
                            (UBaseType_t) TASK_PRIORITY,
                            (TaskHandle_t *) &start_task_handle );
    //2.启动调度器，会自动创建空闲函数
    vTaskStartScheduler();
}

void task_start(void*pv)
{
    taskENTER_CRITICAL();

     xTaskCreate( (TaskFunction_t) task1,
                        (char *)  "task1", 
                        (configSTACK_DEPTH_TYPE) TASK1_STACK,
                        (void *) NULL,
                        (UBaseType_t) TASK1_PRIORITY,
                        (TaskHandle_t *) &task1_handle );
    xTaskCreate( (TaskFunction_t) task2,
                        (char *)  "task2", 
                        (configSTACK_DEPTH_TYPE) TASK2_STACK,
                        (void *) NULL,
                        (UBaseType_t) TASK2_PRIORITY,
                        (TaskHandle_t *) &task2_handle );

    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
     
}


void task1(void*pv)
{
    BaseType_t ret;
    uint8_t satella[]="satella";
    BaseType_t flag;
    char*ps=satella;
    while (1)
    {
        ret=xQueueSend(queue1,&satella,portMAX_DELAY);
        if(ret==pdTRUE)
        {
            printf("send success!\r\n");
        }
        else
        {
             printf("send fail!\r\n");
        }
        
        flag=xQueueSend(big_queue,&ps,portMAX_DELAY);
        if(flag==pdTRUE)
        {
            printf("send big success!\r\n");
        }
        else
        {
             printf("send big fail!\r\n");
        }
    }
    
   
}

void task2(void*pv)
{
    uint8_t buffer=0;
    UBaseType_t ret=0;
    while(1)
    {
        ret=xQueueReceive(queue1,&buffer,portMAX_DELAY);
        if(ret==pdTRUE)
        {
            printf("recive success!\r\n");
        }
        else
        {
             printf("recive fail!\r\n");
        }
        
        ret=xQueueSend(big_queue,&buffer,portMAX_DELAY);
        if(ret==pdTRUE)
        {
            printf("recive big success!\r\n");
        }
        else
        {
             printf("recive big fail!\r\n");
        }
    }
}


