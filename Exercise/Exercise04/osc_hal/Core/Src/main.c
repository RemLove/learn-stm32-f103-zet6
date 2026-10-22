/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
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

/* USER CODE BEGIN PV */
#define BUF_LEN 1024u
static uint16_t adc_buf[BUF_LEN];
static volatile uint32_t frame_cnt=0;

#define HALF_LEN 512u
static uint16_t work[HALF_LEN];
static volatile uint16_t work_fresh=0;


static uint16_t v_max, v_min, v_mid;     /* 本帧最高/最低/中线(ADC原始值) */
static uint16_t trig_i;                  /* 找到的画图起点下标 */
static uint8_t  trig_ok;                 /* 1=本帧找到起点(TRIG) 0=没找到(FREE) */
static volatile uint32_t trig_cnt = 0;   /* 找到起点的累计帧数(观察触发成功率) */

#define TRIG_HYST 20u    /* 迟滞: 穿线要求跨过 mid±20(约16mV), 滤掉噪声假触发 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Scope_ProcessFrame(void)
{
  //1.如果标志位置为 1 buf缓冲期前半部分满了，2 buf缓冲期后半部分满了
  if(work_fresh==1) memcpy(work,&adc_buf[0],HALF_LEN*2);
  else if(work_fresh==2) memcpy(work,&adc_buf[HALF_LEN],HALF_LEN*2);
  else return;
  work_fresh=0;//消耗标志位
  //2.计算像素点的最高点 最低点 中线
  uint16_t vmax=0,vmin=4095;//取不可能值保证第一次赋值就能更新
  for(uint16_t i=0;i<HALF_LEN;i++)
  {
    if(vmax<work[i]) vmax=work[i];
    if(vmin>work[i]) vmin=work[i];
  }
  v_max=vmax;
  v_min=vmin;
  v_mid=(v_max+v_min)/2;
  //3.一次采样512个点 屏幕上只需要画320个点，前一百多个点用来寻找画图起点，找穿过中线第一个位置，没帧都从同一相位开始画
  trig_ok = 0;                          /* 先假设找不到(直流/无信号 -> FREE) */
  trig_i  = 0;                          /* 找不到就从0开始画(波形会滚, 但不卡死) */
	
	uint16_t up = v_mid + TRIG_HYST;                            /* 上门槛 */
  uint16_t dn = (v_mid > TRIG_HYST) ? (v_mid - TRIG_HYST) : 0; /* 下门槛(防下溢) */
	
  for (uint16_t i = 1; i + 320 < HALF_LEN; i++)
  {
    if (work[i - 1] < dn && work[i] >= up)
    {
      trig_i  = i;                      /* 命中: 这一帧的画图起点 */
      trig_ok = 1;
      trig_cnt++;                       /* 成功触发+1 (和frame_cnt对比看成功率) */
      break;                            /* 只要第一个, 后面的过零点不要 */
    }
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM6_Init();
  MX_DAC_Init();
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)adc_buf,BUF_LEN);
  HAL_TIM_Base_Start(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
		Scope_ProcessFrame();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{ 
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
 void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
 {
  if(hadc->Instance==ADC1)
  {
    frame_cnt++;
    work_fresh=1;
  } 

 }
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    frame_cnt++;
    work_fresh=2;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
