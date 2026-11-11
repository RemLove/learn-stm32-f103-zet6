/* 当前文件职责：创建acquire任务 */
#include "app_task.h"
#include "app_object.h"
#include "main.h"
#include "adc.h"
#include "tim.h"
/* DMA 一次循环搬运的样本数 */
#define ACQUIRE_BUF_LEN     ( 1 )

/* 临时打印分频：100Hz / 100 = 1行/秒（阶段3接上显示后整段删除） */
#define ACQUIRE_PRINT_EVERY ( 100 )
//缓冲区
static uint16_t s_adc_raw[ ACQUIRE_BUF_LEN ];
static uint32_t s_seq = 0;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  BaseType_t pxHigherPriorityTaskWoken=pdFALSE;

  if(hadc->Instance==ADC1)
  {
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
        //1.等待信号量
        xSemaphoreTake(xSemDmaDone,portMAX_DELAY);
        //2.赋值
        sample_t s;
        s.raw_adc=s_adc_raw[0];
        s.mv=(uint16_t)( (uint32_t)s_adc_raw[0] * 3300U / 4095U );
        s.seq=++s_seq;
        s.tick_ms=xTaskGetTickCount();
       /* 3.分发：0 超时，满则丢弃+计数 —— 生产者绝不等待消费者 */
        if( xQueueSend( qDisplay, &s, 0 ) != pdPASS )   /* ← 必修二 */
        {
            g_dropDisplay++;                            /* 丢帧计数，不是心跳 */
        }
        if( xQueueSend( qStorage, &s, 0 ) != pdPASS )
        {
            g_dropStorage++;
        }
        //4.采集心跳也活着的
        g_aliveAcquire++;
         //5.临时验证打印（阶段3整段删除）
        if( ( s.seq % ACQUIRE_PRINT_EVERY ) == 0U )
        {
            App_Log( "[acq] seq=%lu tick=%lu raw=%4u mv=%4u\r\n",
                     (unsigned long)s.seq,
                     (unsigned long)s.tick_ms,
                     (unsigned int)s.raw_adc,
                     (unsigned int)s.mv );
        }
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
