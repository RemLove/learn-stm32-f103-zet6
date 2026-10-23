#include "freertos_demo.h"

#include "gpio.h"

//练习一 三个任务 led1 间隔200ms闪烁 led2 间隔500ms闪烁 key0 按下删除led1任务

#define TASK_STACK_DEPTH 128//栈深度128字   128*4字节
#define TASK_PRIORITY 1//任务优先级
//任务一
void led0_task(void*pvParameters)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}
TaskHandle_t led0_handle;

//任务二
void led1_task(void*pvParameters)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
}
TaskHandle_t led1_handle;

//任务三
static volatile uint8_t key_flag=0;
static uint8_t key_last=GPIO_PIN_SET;//默认高电平
static uint32_t key_time=0;

void key0_task(void*pvParameters)
{
    for (;;)
    {
        //如果电平变化
        if(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)!=key_last)
        {
            key_last=HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);
            key_time=HAL_GetTick();
        }
        if(HAL_GetTick()-key_time>50)//真变化了
        {
            //看是否是有效电平
            if((key_last==GPIO_PIN_RESET)&&(led0_handle!=NULL))
            {
                vTaskDelete(led0_handle);
                led0_handle = NULL;
                key_time=HAL_GetTick();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
TaskHandle_t key0_handle=NULL;

//任务四:这个任务用来启动三个任务,用完就删
TaskHandle_t task_start_handle;
void task_start(void*pvParameters)
{

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
                            TASK_PRIORITY,
                            &led1_handle
                            );  
                            
    xTaskCreate(key0_task,
                        "key0_task", 
                        TASK_STACK_DEPTH,
                        NULL,
                        TASK_PRIORITY,
                        &key0_handle
                        );    
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
