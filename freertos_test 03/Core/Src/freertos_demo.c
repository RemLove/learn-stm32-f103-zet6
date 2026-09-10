
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_demo.h"
#include "stm32f1xx_hal.h"
#include "usart.h"
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


void FreeRtos_Start(void)
{
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

    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
     
}



void task1(void*pv)
{
    UBaseType_t task_priority=0;
    task_priority = uxTaskPriorityGet(task1_handle);
    printf("task_priority:%d\r\n",task_priority);
    vTaskPrioritySet(task1_handle,1);
    task_priority = uxTaskPriorityGet(task1_handle);
    printf("task_priority:%d\r\n",task_priority);
    vTaskDelete(NULL);
}


