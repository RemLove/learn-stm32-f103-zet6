/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"usart.h"
#include"queue.h"
#include "adc.h"//#include "semaphore.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
  uint16_t data;//ԭʼֵ��adc��ʮ��λ��0��4095
  float val;//ת�����ֵ��0��3.3v
}SensorData_t;//�ɼ��¼�
//�������¼����ǰ����¼�
typedef enum{Sensor_event=0,Key_event}Event_Type;
//ϸ�ְ����¼�
typedef enum{Key_None=0,Key_Short,Key_Long,Key_WakeUp}Key_Event;
//����ģʽ���Զ������ֶ�
typedef enum{Mode_Auto=0,Mode_Manual}Mode_Type;
typedef struct 
{
  Event_Type type;
  SensorData_t sd;
  Key_Event key;
}Event_t;
typedef struct 
{
  uint16_t threshold;//������ֵ
  uint8_t mode;//����ģʽ
  uint8_t alarmE;//�����Ƿ�ʹ��
}Cfg_t;
Cfg_t g_cfg = { 2500, Mode_Auto, 1 };
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef enum{S_Idle=0,S_Down,S_Long}Key_status;
static uint8_t key_stable=1;//�ȶ�ʱ�ĵ�ƽ
static uint8_t key_last=1;//�ϴεĵ�ƽ
static uint32_t key_time=0;//�жϵ�ƽ�Ƿ��ȶ���ʱ��
static uint32_t key_time_down=0;//�жϳ������Ƕ̰���ʱ��
static Key_status key_status=S_Idle;
Key_Event Key_Scan(void)
{
  Key_Event ev=Key_None;
  //1.��������
  uint8_t key_now=HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);//���ڵĵ�ƽ
  if(key_last!=key_now)//�����ƽ�仯
  {
    key_last=key_now;
    key_time=HAL_GetTick();
  }
  if(HAL_GetTick()-key_time>=10)//��ƽ�ȶ�10ms���Ѿ�����
  {
    key_stable=key_now;
    key_time=HAL_GetTick();
  }
  //2.״̬��
  switch (key_status)
  {
  case S_Idle:
    if(key_stable==0)//�������
    {
      ev=Key_None;
      key_status=S_Down;
      key_time_down=HAL_GetTick();
    }
    break;
  case S_Down:
    if(key_stable==1)//��������֣��Ƕ̰�
    {
      ev=Key_Short;
      key_status=S_Idle;
    }
    else if(HAL_GetTick()-key_time_down>=1000)
    {
      ev=Key_Long;
      key_status=S_Long;
    }
  break;
  case S_Long:
    if(key_stable==1)//�����������
    {
      ev=Key_None;
      key_status=S_Idle;
    }
    break;
  }
  return ev;
}

static uint8_t WK_last=0;
static uint8_t WK_stable=0;
static uint32_t WK_time=0;
static uint8_t  wk_locked=0;// ���Ѱ��¡���������ֻʶ������һ˲��
Key_Event WK_Scan(void)
{
  Key_Event ev=Key_None;
  //1.����������Ǹߵ�ƽ��Ч
  uint8_t wk_now=HAL_GPIO_ReadPin(WK_UP_GPIO_Port,WK_UP_Pin);
  if(WK_last!=wk_now)
  {
    WK_last=wk_now;
    WK_time=HAL_GetTick();
  }
  if(HAL_GetTick()-WK_time>=10)
  {
    WK_stable=wk_now;
    WK_time=HAL_GetTick();
  }
  if(WK_stable==1 && wk_locked==0){
      wk_locked=1;          // ��������
      ev=Key_WakeUp;          // ����һ�ε���
    }
    if(WK_stable==0) wk_locked=0; // ���ֺ�������´ΰ������ٴ���
    return ev;
}

#define RB_SIZE 128
static uint8_t RingBiffer[RB_SIZE];
static uint16_t head=0;
static uint16_t tail=0;

void RB_Push(uint8_t b)
{
  uint16_t next=(head+1)%RB_SIZE;
  if(next!=tail)
  {
    RingBiffer[head]=b;
    head=tail;

  }
}

int RB_Pop(void)
{
  if(head==tail)return -1;
  uint8_t ret=RingBiffer[tail];
  tail=(tail+1)%RB_SIZE;
  return ret;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

//SensorData_t sd;
QueueHandle_t Queue_Handle;
SemaphoreHandle_t Semaphore_handle;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTaskLED */
osThreadId_t myTaskLEDHandle;
const osThreadAttr_t myTaskLED_attributes = {
  .name = "myTaskLED",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskSensor */
osThreadId_t myTaskSensorHandle;
const osThreadAttr_t myTaskSensor_attributes = {
  .name = "myTaskSensor",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for myTaskControl */
osThreadId_t myTaskControlHandle;
const osThreadAttr_t myTaskControl_attributes = {
  .name = "myTaskControl",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for myTaskKey */
osThreadId_t myTaskKeyHandle;
const osThreadAttr_t myTaskKey_attributes = {
  .name = "myTaskKey",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for myTaskCmd */
osThreadId_t myTaskCmdHandle;
const osThreadAttr_t myTaskCmd_attributes = {
  .name = "myTaskCmd",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTasLED(void *argument);
void StartTaskSensor(void *argument);
void StartTaskControl(void *argument);
void StartTaskKey(void *argument);
void StartTaskCmd(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  Queue_Handle = xQueueCreate(8, sizeof(Event_t));

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    Semaphore_handle = xSemaphoreCreateBinary();  // 二值信号量，创建后初始是“空”的，任务先Take阻塞等
/* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTaskLED */
  myTaskLEDHandle = osThreadNew(StartTasLED, NULL, &myTaskLED_attributes);

  /* creation of myTaskSensor */
  myTaskSensorHandle = osThreadNew(StartTaskSensor, NULL, &myTaskSensor_attributes);

  /* creation of myTaskControl */
  myTaskControlHandle = osThreadNew(StartTaskControl, NULL, &myTaskControl_attributes);

  /* creation of myTaskKey */
  myTaskKeyHandle = osThreadNew(StartTaskKey, NULL, &myTaskKey_attributes);

  /* creation of myTaskCmd */
  myTaskCmdHandle = osThreadNew(StartTaskCmd, NULL, &myTaskCmd_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTasLED */
/**
* @brief Function implementing the myTaskLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTasLED */
void StartTasLED(void *argument)
{
  /* USER CODE BEGIN StartTasLED */
  /* Infinite loop */
  TickType_t now_time=xTaskGetTickCount();
  for(;;)
  {
    
    HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
    //osDelay(1);
    vTaskDelayUntil(&now_time,500);
  }
  /* USER CODE END StartTasLED */
}

/* USER CODE BEGIN Header_StartTaskSensor */
/**
* @brief Function implementing the myTaskSensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskSensor */
void StartTaskSensor(void *argument)
{
  /* USER CODE BEGIN StartTaskSensor */
  /* Infinite loop */
  Event_t ev;
  TickType_t time = xTaskGetTickCount();
  uint32_t sum=0;
  for(;;)
  {
    sum = 0;
    //�ɼ��Ĵ���ƽ��ֵ
    for (size_t i = 0; i < 4; i++)
    {
      HAL_ADC_Start(&hadc1);
      HAL_ADC_PollForConversion(&hadc1,10);
      sum += HAL_ADC_GetValue(&hadc1);
      HAL_ADC_Stop(&hadc1);
    }
    ev.sd.data=sum/4;
    ev.sd.val=ev.sd.data*(3.3f/4095.0f);
    ev.type=Sensor_event;
    ev.key=Key_None;
    xQueueSend(Queue_Handle,&ev,10);
    vTaskDelayUntil(&time,100);//100ms�ɼ�һ��
  }
  
  /* USER CODE END StartTaskSensor */
}

/* USER CODE BEGIN Header_StartTaskControl */
/**
* @brief Function implementing the myTaskControl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskControl */
void StartTaskControl(void *argument)
{
  /* USER CODE BEGIN StartTaskControl */
  /* Infinite loop */
  Event_t ev;
  for(;;)
  {
    if(xQueueReceive(Queue_Handle,&ev,portMAX_DELAY)==pdTRUE)//������յ����ݣ��ж���˭��
    {
      if(ev.type==Sensor_event)
      {
        printf("raw=%4d %dmV | ģʽ:%s ����:%s ��ֵ:%d\r\n",
                    ev.sd.data,
                    (int)((uint32_t)ev.sd.data * 3300UL / 4095UL),
                    g_cfg.mode==Mode_Auto?"�Զ�":"�ֶ�",   // ö��ת������ʾ
                    g_cfg.alarmE?"��":"��",
                    g_cfg.threshold);
      }
      else if(ev.type==Key_event)
      {
        switch (ev.key)
        {
          case Key_Short:
          {
            printf("key short\r\n");
            g_cfg.mode=(g_cfg.mode==Mode_Auto)?Mode_Manual:Mode_Auto;
          }
          break;
          case Key_Long:
            g_cfg.alarmE=!g_cfg.alarmE;
            printf(">> ����ʹ��:%s\r\n",g_cfg.alarmE?"��":"��");
          break;
          case Key_WakeUp:         // WK����ֵ+500
             g_cfg.threshold+=500;
            if(g_cfg.threshold>4095) g_cfg.threshold=0; //�����̻��Ƶ�0
            printf(">> ��ֵ=%d\r\n",g_cfg.threshold);
          break;
        }
      }
    }
  }
  /* USER CODE END StartTaskControl */
}

/* USER CODE BEGIN Header_StartTaskKey */
/**
* @brief Function implementing the myTaskKey thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskKey */
void StartTaskKey(void *argument)
{
  /* USER CODE BEGIN StartTaskKey */
  /* Infinite loop */
  TickType_t time = xTaskGetTickCount();
  Event_t ev;
  Key_Event key=Key_None;
  for(;;)
  {
    key=Key_Scan();
    if(key==Key_None) key=WK_Scan();
    if(key!=Key_None)
    {
      ev.type=Key_event;
      ev.key=key;
      xQueueSend(Queue_Handle,&ev,0);
    }
    vTaskDelayUntil(&time,20);
  }
  
  /* USER CODE END StartTaskKey */
}

/* USER CODE BEGIN Header_StartTaskCmd */
/**
* @brief Function implementing the myTaskCmd thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskCmd */
void StartTaskCmd(void *argument)
{
  /* USER CODE BEGIN StartTaskCmd */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTaskCmd */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

