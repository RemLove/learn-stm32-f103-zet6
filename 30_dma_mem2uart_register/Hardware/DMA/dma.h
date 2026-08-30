#ifndef __DMA_H__
#define __DMA_H__

#include"stm32f10x.h"

void DMA1_Init(void);
void DMA1_Transfer(uint32_t srcAddr, uint32_t dstAddr, uint16_t size);

#endif 
