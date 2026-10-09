#include "spi.h"

void SPI_Init(void)
{
    //1.时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    //1.1.配置GPIO PB12通用推挽输出MODE11 CNF00
    GPIOB->CRH|= (GPIO_CRH_MODE12);
    GPIOB->CRH&= ~(GPIO_CRH_CNF12);
    //PB13 15复用推挽输出MODE11 CNF10
    GPIOB->CRH|= (GPIO_CRH_MODE13);
    GPIOB->CRH|= (GPIO_CRH_CNF13_1);
    GPIOB->CRH&= ~(GPIO_CRH_CNF13_0);
    GPIOB->CRH|= (GPIO_CRH_MODE15);
    GPIOB->CRH|= (GPIO_CRH_CNF15_1);
    GPIOB->CRH&= ~(GPIO_CRH_CNF15_0);
    //PB14浮空输入模式MODE00 CNF01
    GPIOB->CRH&= ~(GPIO_CRH_MODE14);
    GPIOB->CRH|= (GPIO_CRH_CNF14_0);
    GPIOB->CRH&= ~(GPIO_CRH_CNF14_1);

    //2.配置SPI
   //2.1配置SPI2_CR1寄存器
    SPI2->CR1 |= SPI_CR1_MSTR; //主机模式
    //2.2软件配置片选
    SPI2->CR1 |= SPI_CR1_SSM; //软件片选
    SPI2->CR1 |= SPI_CR1_SSI; //软件片选有效
    //2.3极性和相位
    SPI2->CR1 &= ~SPI_CR1_CPOL; //空闲低电平
    SPI2->CR1 &= ~SPI_CR1_CPHA; //第一个时钟沿采样数据
    //2.4配置波特率 四分频 001
    SPI2->CR1 &= ~SPI_CR1_BR;
    SPI2->CR1 |= SPI_CR1_BR_0; // fPCLK/4
    //2.5配置数据帧格式 8位数据帧
    SPI2->CR1 &= ~SPI_CR1_DFF; //8位数据帧
    //2.6高位先行
    SPI2->CR1 &= ~SPI_CR1_LSBFIRST; //高位先行
    //2.7使能SPI
    SPI2->CR1 |= SPI_CR1_SPE;
    //2.8片选空闲电平拉高（GPIO 复位后 ODR=0，CS 会是低电平，先置高）
    CS_HIGH();
}
void SPI_Start(void)
{
    CS_LOW();
}
void SPI_Stop(void)
{
    CS_HIGH();
}

uint8_t SPI_SwapByte(uint8_t byte)
{
    //1.发送数据
    SPI2->DR = byte;
    //2.等待发送完成
    while(!(SPI2->SR & SPI_SR_TXE));
    //3.等待接收完成
    while(!(SPI2->SR & SPI_SR_RXNE));
    //4.返回接收到的数据
    return (uint8_t)(SPI2->DR&0xFF);
}

// 供 W5500 库回调使用：读一字节（SPI 全双工，发 0xFF 顺便收一字节）
uint8_t SPI_ReadByte(void)
{
    return SPI_SwapByte(0xFF);
}

// 供 W5500 库回调使用：写一字节（丢弃返回）
void SPI_WriteByte(uint8_t byte)
{
    (void)SPI_SwapByte(byte);
}

