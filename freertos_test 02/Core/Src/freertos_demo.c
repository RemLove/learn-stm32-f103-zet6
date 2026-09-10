
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

//任务2的配置
#define TASK2_STACK 128
#define TASK2_PRIORITY 3
TaskHandle_t task2_handle;
void task2(void*pv);

//任务3的配置
#define TASK3_STACK 128
#define TASK3_PRIORITY 4
TaskHandle_t task3_handle;
void task3(void*pv);






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
    xTaskCreate( (TaskFunction_t) task2,
                        (char *)  "task2", 
                        (configSTACK_DEPTH_TYPE) TASK2_STACK,
                        (void *) NULL,
                        (UBaseType_t) TASK2_PRIORITY,
                        (TaskHandle_t *) &task2_handle );
    xTaskCreate( (TaskFunction_t) task3,
                        (char *)  "task3", 
                        (configSTACK_DEPTH_TYPE) TASK3_STACK,
                        (void *) NULL,
                        (UBaseType_t) TASK3_PRIORITY,
                        (TaskHandle_t *) &task3_handle );

    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
     
}


//LED0 500ms反转
void task1(void*pv)
{
   while (1)
   {
    printf("task1 \r\n");
    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
    vTaskDelay(500);
   }
   
   
}

//LED1 500ms反转
void task2(void*pv)
{
    while (1)
   {
    printf("task2 \r\n");
    HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_5);
    vTaskDelay(500);
   }
   
   
}

//KEY0按键按下 任务1结束
char task_info[500];
void task3(void*pv)
{
    while(1)
    {
        HAL_Delay(10);
        if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET)
        {
            printf("suspend task1!\n\r");
            vTaskSuspend(task1_handle);
        }
        if(HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
        {
            printf("resume task1!\n\r");
           vTaskResume(task1_handle);
        }
         vTaskList(task_info);
        printf("%s\r\n",task_info);
    }
}

