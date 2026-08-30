#include "dma.h"


void DMA1_Init(void)
{
    //1.时钟
    RCC->AHBENR|=RCC_AHBENR_DMA1EN;
    //2.配置DMA1通道4: 内存→USART1_TX
    // DIR=1: 从内存读取, 写入外设
    DMA1_Channel4->CCR|=DMA_CCR4_DIR;
    // 数据宽度: 8位
    DMA1_Channel4->CCR&=~(DMA_CCR4_MSIZE|DMA_CCR4_PSIZE);
    // 内存地址递增
    DMA1_Channel4->CCR|=DMA_CCR4_MINC;
    //3.配置DMA1通道的中断
    DMA1_Channel4->CCR|=DMA_CCR4_TCIE;
    //开启串口的DMA发送请求
    USART1->CR3|=USART_CR3_DMAT;

    //4.打开NVIC中断
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(DMA1_Channel4_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);

}

void DMA1_Transfer(uint32_t srcAddr, uint32_t dstAddr, uint16_t size)
{
    // Transfer code for DMA1
    //1.配置外设地址
    DMA1_Channel4->CPAR=dstAddr;
    //2.配置内存地址
    DMA1_Channel4->CMAR=srcAddr;
    //3.配置传输数量
    DMA1_Channel4->CNDTR=size;
    //4.使能DMA1通道
    DMA1_Channel4->CCR|=DMA_CCR4_EN;
}
//中断服务程序
void DMA1_Channel4_IRQHandler(void)
{
    // Check for transfer complete interrupt
    if (DMA1->ISR & DMA_ISR_TCIF4) {
        DMA1->IFCR |= DMA_IFCR_CTCIF4;
        DMA1_Channel4->CCR&=~DMA_CCR4_EN;
    }
}
