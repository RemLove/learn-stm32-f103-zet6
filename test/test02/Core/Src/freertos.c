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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskKEY */
osThreadId_t myTaskKEYHandle;
const osThreadAttr_t myTaskKEY_attributes = {
  .name = "myTaskKEY",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskPrint */
osThreadId_t myTaskPrintHandle;
const osThreadAttr_t myTaskPrint_attributes = {
  .name = "myTaskPrint",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myBinarySem01 */
osSemaphoreId_t myBinarySem01Handle;//信号量
const osSemaphoreAttr_t myBinarySem01_attributes = {
  .name = "myBinarySem01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskLED(void *argument);
void StartTaskKEY(void *argument);
void StartTaskPrint(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of myBinarySem01 */
  myBinarySem01Handle = osSemaphoreNew(1, 0, &myBinarySem01_attributes);

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
  myTaskLEDHandle = osThreadNew(StartTaskLED, NULL, &myTaskLED_attributes);

  /* creation of myTaskKEY */
  myTaskKEYHandle = osThreadNew(StartTaskKEY, NULL, &myTaskKEY_attributes);

  /* creation of myTaskPrint */
  myTaskPrintHandle = osThreadNew(StartTaskPrint, NULL, &myTaskPrint_attributes);

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

/* USER CODE BEGIN Header_StartTaskLED */
/**
* @brief Function implementing the myTaskLED thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskLED */
void StartTaskLED(void *argument)
{
  /* USER CODE BEGIN StartTaskLED */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED0_GPIO_Port,LED0_Pin);
    osDelay(500);
  }
  /* USER CODE END StartTaskLED */
}

/* USER CODE BEGIN Header_StartTaskKEY */
/**
* @brief Function implementing the myTaskKEY thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskKEY */
void StartTaskKEY(void *argument)
{
  /* USER CODE BEGIN StartTaskKEY */
  /* Infinite loop */

  for(;;)
  {
    if(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)==GPIO_PIN_RESET)
    {
      osDelay(20);
      if(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)==GPIO_PIN_RESET)
      {
        //如果按键按下，释放信号量，开始打印
        printf("key press!");
        osSemaphoreRelease(myBinarySem01Handle);
        osDelay(20);
      }
    }
    osDelay(1);
  }
  /* USER CODE END StartTaskKEY */
}

/* USER CODE BEGIN Header_StartTaskPrint */
/**
* @brief Function implementing the myTaskPrint thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskPrint */
void StartTaskPrint(void *argument)
{
  /* USER CODE BEGIN StartTaskPrint */
  /* Infinite loop */

  for(;;)
  {
    if(osSemaphoreAcquire(myBinarySem01Handle,1000)==pdTRUE)
    {
      printf("get semaphore\r\n");
    }
    osDelay(2000);
  }
  /* USER CODE END StartTaskPrint */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
//利用TickHook实现呼吸灯 LED0
static volatile uint32_t cnt=0;//频率计数
static volatile uint32_t duty=0;//占空比
static volatile int32_t dir=1;//方向 1变亮，-1变灭
static volatile uint32_t BreathCnt=0;//每一个duty中有100ms，100ms才允许duty变化一次
//0-10, 亮度被切换成10档，100ms变化一次亮度。从灭到亮需要1000ms
void vApplicationTickHook (void) 
{ 
  //1.快循环   10ms一次改变
  if(cnt<duty)
  {
    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);
  }
  else 
  {
    HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);
  }
  cnt++;
  if(cnt>=10)
  {
    cnt=0;
  }
  //2.慢循环
  BreathCnt++;
  if(dir==1)//如果是要变亮
  {
    if(BreathCnt>=100)
    {
      BreathCnt=0;
      if(duty<=10)
      {
        duty++;
      }
      else
      {
        dir=-1;
      }
    }
  }
  else//如果是要变灭
  {
    if(BreathCnt>=100)
    {
      BreathCnt=0;
      if(duty>0)
      {
        duty--;
      }
      else
      {
        dir=1;
      }
    }
  }
  
}
/* USER CODE END Application */

