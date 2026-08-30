#include "dma.h"


void DMA1_Init(void)
{
    // Initialization code for DMA1
    //1.时钟
    RCC->AHBENR|=RCC_AHBENR_DMA1EN;
    //2.配置DMA1通道 地址 方向 数据 增量 
    DMA1_Channel1->CCR|=DMA_CCR1_MEM2MEM;

    DMA1_Channel1->CCR&=~DMA_CCR1_DIR;

    DMA1_Channel1->CCR&=~DMA_CCR1_MSIZE;//八位
    DMA1_Channel1->CCR&=~DMA_CCR1_PSIZE;//八位

    DMA1_Channel1->CCR|=DMA_CCR1_MINC;
    DMA1_Channel1->CCR|=DMA_CCR1_PINC;
    //3.配置DMA1通道的中断  
    DMA1_Channel1->CCR|=DMA_CCR1_TCIE;
    DMA1_Channel1->CCR|=DMA_CCR1_TEIE;
    //4.打开NVIC中断
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

void DMA1_Transfer(uint32_t srcAddr, uint32_t dstAddr, uint16_t size)
{
    // Transfer code for DMA1
    //1.配置源地址
    DMA1_Channel1->CPAR=srcAddr;
    //2.配置目标地址
    DMA1_Channel1->CMAR=dstAddr;
    //3.配置传输数量
    DMA1_Channel1->CNDTR=size;
    //4.使能DMA1通道
    DMA1_Channel1->CCR|=DMA_CCR1_EN;
}
//中断服务程序
void DMA1_Channel1_IRQHandler(void)
{
    // Check for transfer complete interrupt
    if (DMA1->ISR & DMA_ISR_TCIF1) {
        flag=1;
        // Clear the transfer complete flag

        DMA1->IFCR |= DMA_IFCR_CTCIF1;
        // Handle the transfer complete event (e.g., notify the application)
        //关闭DMA1通道
        DMA1_Channel1->CCR&=~DMA_CCR1_EN;  
    }
    // Check for transfer error interrupt
    if (DMA1->ISR & DMA_ISR_TEIF1) {
        // Clear the transfer error flag
        DMA1->IFCR |= DMA_IFCR_CTEIF1;
        // Handle the transfer error event (e.g., notify the application)
        //关闭DMA1通道
        DMA1_Channel1->CCR&=~DMA_CCR1_EN;  
    }
}
