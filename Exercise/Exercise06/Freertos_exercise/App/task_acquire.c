/* 当前文件职责：创建acquire任务 */
#include "app_task.h"
#include "app_object.h"
#include "app_main.h"      /* App_Log 原型在这里；缺它=隐式声明警告，且返回值不可信 */
#include "main.h"
#include "adc.h"
#include "tim.h"
/* DMA 一次循环搬运的样本数 */
#define ACQUIRE_BUF_LEN     ( 1 )

/* 临时打印分频：100Hz / 100 = 1行/秒（阶段3接上显示后整段删除） */
#define ACQUIRE_PRINT_EVERY ( 100 )
//缓冲区
static uint16_t s_adc_raw[ ACQUIRE_BUF_LEN ];

/* ★ 采样序号必须由中断累加，不能由任务累加
 * 任务累加 = 统计"任务被唤醒了几次"，丢帧时序号是连号，缺口被抹掉；
 * 中断累加 = 统计"硬件真正采了多少次"，丢一帧就缺一个号，缺口看得见。
 * 顺带在中断里把 raw 和 seq 一起锁存，任务一次性读走副本：
 * 读窗口只有几个周期，100Hz 的采样想插进来几乎不可能；
 * 真错位了也只是"上一帧电压配了新序号"，波形上根本看不出来。*/
static volatile uint32_t s_hw_seq = 0;
static volatile uint16_t s_hw_raw = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  BaseType_t pxHigherPriorityTaskWoken=pdFALSE;

  if(hadc->Instance==ADC1)
  {
    s_hw_raw = s_adc_raw[0];      /* 先锁数据 */
    s_hw_seq++;                   /* 再递增序号，成对更新 */
    xSemaphoreGiveFromISR(xSemDmaDone,&pxHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );//如果释放信号量后有更高优先级的任务被唤醒，则立即切换到该任务 保证实时性
}
static void TaskAcquire( void *pv )
{
    (void)pv;
    HAL_ADC_Start_DMA(&hadc1,(uint32_t*)s_adc_raw,ACQUIRE_BUF_LEN);
    HAL_TIM_Base_Start(&htim3);
    for (;;)
    {
        /* 1.等信号量 —— 必须带超时，不能 portMAX_DELAY
         * 用 portMAX_DELAY 的话，ADC 中断一旦不来（TIM3 没启、通道配错），
         * 这个任务就永远卡在这一句，循环末尾的心跳永远不执行 →
         * monitor 判它"冻死" → 不喂狗 → 2s 复位 → 复位后还是没中断 → 无限重启。
         * 带超时后：100ms 一定能返回，不管有没有数据都转一圈把心跳打上。*/
        if( xSemaphoreTake( xSemDmaDone, pdMS_TO_TICKS( APP_WAIT_MS ) ) == pdPASS )
        {
            /* 2.赋值 */
            sample_t s = { 0 };                 /* ★ 全零初始化：reserved 忘了赋值的话，
                                                   写进 Flash 的是栈上的随机垃圾 */
            uint16_t raw = s_hw_raw;            /* 只读一次，别在中途被中断改掉 */
            s.raw_adc = raw;
            s.mv      = (uint16_t)( (uint32_t)raw * 3300U / 4095U );
            s.seq     = s_hw_seq;               /* 硬件真实采样序号 */
            s.tick_ms = xTaskGetTickCount();

            /* 3.分发：0 超时，满则丢弃+计数 —— 生产者绝不等待消费者 */
            if( xQueueSend( qDisplay, &s, 0 ) != pdPASS )   /* ← 必修二 */
            {
                g_dropDisplay++;                            /* 丢帧计数，不是心跳 */
            }
            if( xQueueSend( qStorage, &s, 0 ) != pdPASS )
            {
                g_dropStorage++;
            }

            /* 4.业务量：真的搬走了一个样本才算 */
            g_workAcquire++;

            /* 5.临时验证打印（阶段3整段删除） */
            if( ( s.seq % ACQUIRE_PRINT_EVERY ) == 0U )
            {
                App_Log( "[acq] seq=%lu tick=%lu raw=%4u mv=%4u\r\n",
                         (unsigned long)s.seq,
                         (unsigned long)s.tick_ms,
                         (unsigned int)s.raw_adc,
                         (unsigned int)s.mv );
            }
        }
        /* 6.心跳：放在 if 外面，无条件 +1
         * 这一句的位置是整个看门狗设计的关键 —— 它证明的是"任务还在转"，
         * 而不是"拿到了数据"。没数据也要活着。*/
        g_aliveAcquire++;
    }
    

}
TaskHandle_t xTaskAcquire=NULL;
/* 创建acquire任务 */
void TaskAcquire_Create(void)
{
    BaseType_t ok= xTaskCreate(TaskAcquire,
                "TaskAcquire",
                TASK_STACK_ACQUIRE,
                NULL,
                TASK_PRIO_ACQUIRE,
                &xTaskAcquire);
    configASSERT( ok == pdPASS );
}
