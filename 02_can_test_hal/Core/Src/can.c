/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 36;
  hcan.Init.Mode = CAN_MODE_SILENT_LOOPBACK;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_8TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = ENABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void FilterConfig(void)
{
  CAN_FilterTypeDef sFilterConfig;
  //1.接收用FIFO0
  sFilterConfig.FilterFIFOAssignment=CAN_RX_FIFO0;
  //2.用过滤组0
  sFilterConfig.FilterBank=0;
  //3.模式用掩码模式
  sFilterConfig.FilterMode=CAN_FILTERMODE_IDMASK;
  //4.用32位字宽
  sFilterConfig.FilterScale=CAN_FILTERSCALE_32BIT;
  //5.配置id寄存器
  sFilterConfig.FilterIdHigh=0x0000;
  sFilterConfig.FilterIdLow=0x0000;
  //6.配置掩码寄存器
  sFilterConfig.FilterMaskIdHigh=0x0000;
  sFilterConfig.FilterMaskIdLow=0x0000;
  sFilterConfig.FilterActivation=ENABLE;
  HAL_CAN_ConfigFilter(&hcan,&sFilterConfig);
}
//发送报文
void SendMsg(uint16_t stdid,uint8_t* data,uint8_t len)
{
  //1.检查是否有可用邮箱
  while((HAL_CAN_GetTxMailboxesFreeLevel(&hcan))==0)
  {}
  //2.配置报文信息
  CAN_TxHeaderTypeDef Header;
  Header.StdId=stdid;
  Header.DLC=len;
  Header.IDE=CAN_ID_STD;
  Header.RTR=CAN_RTR_DATA;
  //3.发送报文
  uint32_t TxMailbox;
  HAL_CAN_AddTxMessage(&hcan,&Header,data,&TxMailbox);
}
//接收报文
void RxMsg1(RxMsg rxmsg[],uint8_t*msgCount)
{
  //1.判断接收报文个数
  *msgCount=HAL_CAN_GetRxFifoFillLevel(&hcan,CAN_RX_FIFO0);
  //2.循环接收报文
  CAN_RxHeaderTypeDef Header;
  for(uint8_t i=0;i<*msgCount;i++)
  {
    HAL_CAN_GetRxMessage(&hcan,CAN_RX_FIFO0,&Header,rxmsg[i].data);
    rxmsg[i].len=Header.DLC;
    rxmsg[i].stdid=Header.StdId;
  }
}
/* USER CODE END 1 */
