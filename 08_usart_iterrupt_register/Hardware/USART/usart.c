#include"usart.h"

//初始化
void USART_Init(void)
{
    //1.打开GPIO USART时钟
    RCC->APB2ENR|=RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR|=RCC_APB2ENR_USART1EN;
    //2.配置 PA9->T  复用输出推挽   PA10-> R  模拟输入
    GPIOA->CRH|=GPIO_CRH_MODE9;
    GPIOA->CRH|=GPIO_CRH_CNF9_1;
    GPIOA->CRH&=~GPIO_CRH_CNF9_0;

    GPIOA->CRH&=~GPIO_CRH_MODE10;
    GPIOA->CRH&=~GPIO_CRH_CNF10_1;
    GPIOA->CRH|=GPIO_CRH_CNF10_0;
    //3.配置USART
    USART1->BRR=0X271;//波特率115200
    USART1->CR1|=USART_CR1_UE;
    USART1->CR1|=(USART_CR1_RE|USART_CR1_TE);
    //4.打开USART  RXNEIE  IDLEIE中断
    USART1->CR1|=USART_CR1_RXNEIE;
    USART1->CR1|=USART_CR1_IDLEIE;
    //配置NVIC
    NVIC_SetPriorityGrouping(3);
    NVIC_SetPriority(USART1_IRQn,3);
    NVIC_EnableIRQ(USART1_IRQn);
}
//发送一个字符
void USART_SendChar(uint8_t ch)
{
    while((USART1->SR&USART_SR_TXE)==0){}
    USART1->DR=ch;
}
//接收一个字符
uint8_t USART_ReciveChar(void)
{
    while((USART1->SR&USART_SR_RXNE)==0){}
    return USART1->DR;
}
//发送一个字符串
void USART_SendString(uint8_t* str,uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        USART_SendChar(str[i]);
    }
    
}
//接收一个字符串
void USART_ReciveString(uint8_t buff[],uint8_t*size)
{
    uint8_t i=0;
    while(1)
    {
        while((USART1->SR&USART_SR_RXNE)==0)//接收一个数据帧
        {
            if(USART1->SR&USART_SR_IDLE)//发现空闲了
            {
                *size=i;
                return;
            }
        }
        buff[i]=USART1->DR;
        i++;
    }
}
void USART1_IRQHandler(void)
{
    //判断是哪个来的中断
    if(USART1->SR&USART_SR_RXNE)//  接收到一个字符
    {
        buff[size]=USART1->DR;
        size++;
    }
    else if(USART1->SR&USART_SR_IDLE)//接收完成一个字符串
    {
        //清除idle标志位
        USART1->DR;
        //发送到电脑上
        USART_SendString(buff,size);
        size=0;
    }
}
