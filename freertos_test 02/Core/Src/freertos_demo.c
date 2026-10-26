#include "freertos_demo.h"
#include "key.h"
#include "gpio.h"
#include <stdio.h>

/*
 * 练习二 一个任务 按下按键关闭中断 freertos管理的是5-15，两个定时器中断 一个中断4 一个6
 */

#define TASK_STACK_DEPTH 128//栈深度128字   128*4字节
#define TASK_PRIORITY 1//任务优先级

//任务句柄
TaskHandle_t task1_handle=NULL;
TaskHandle_t task2_handle=NULL;
TaskHandle_t task_start_handle=NULL;

//任务一
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

           // printf("evt=%d\r\n", evt);

            switch (evt)          /* 3. 处理的是局部副本 evt，不是 key_evt */
            {
            case KEY_SHORT:
                printf("open interrupt\r\n");
                portENABLE_INTERRUPTS();
                break;
            case KEY_LONG:
            printf("close interrupt\r\n");
                portDISABLE_INTERRUPTS();
                break;
            default:
                break;            /* KEY_UP/KEY_DOWN 打一行 evt 就够了 */
            }
        }
    }
}

//任务二 获取任务一的状态
void task2(void *pvParameters)
{
    //获取任务一的任务优先级 打印 再设置优先级 打印
    UBaseType_t task_priorty=0;
    task_priorty = uxTaskPriorityGet(task1_handle);
    printf("%d\r\n",task_priorty);
    vTaskPrioritySet(task1_handle,4);
    task_priorty = uxTaskPriorityGet(task1_handle);
    printf("%d\r\n",task_priorty);

    //获取任务数量并打印
    UBaseType_t task_number = uxTaskGetNumberOfTasks();
    printf("%d\r\n",task_number);

    //获取所有任务状态
    TaskStatus_t task_arr[10]={0};
    uxTaskGetSystemState(task_arr,task_number,NULL);//最后一个参数是运行时间
    for (uint8_t i = 0; i < task_number; i++)
    {
        printf("%s\t\t%d\t\t%d\t\t%d\t\t%d\t\t",
               task_arr[i].pcTaskName,
               task_arr[i].xTaskNumber,
               task_arr[i].eCurrentState,
               task_arr[i].uxCurrentPriority,
               task_arr[i].uxBasePriority
        );
    }
    //获取单个任务信息
    TaskStatus_t task1_status;//保存获取到的信息
    BaseType_t task1_freestackspace;
    vTaskGetInfo(task1_handle,&task1_status,task1_freestackspace,eInvalid);
    
    for (;;)
    {
        
    }
    
}

//任务三:这个任务用来启动三个任务,用完就删
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



