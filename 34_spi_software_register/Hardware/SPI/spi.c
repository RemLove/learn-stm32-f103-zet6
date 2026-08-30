#include "spi.h"

void SPI_Init(void)
{
    //1.时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    //2.配置GPIO PB12 13 15通用推挽输出MODE11 CNF00 , 14浮空输入MODE00 CNF01
    GPIOB->CRH|= (GPIO_CRH_MODE12 | GPIO_CRH_MODE13 | GPIO_CRH_MODE15);
    GPIOB->CRH&= ~(GPIO_CRH_CNF12 | GPIO_CRH_CNF13 | GPIO_CRH_CNF15);
    GPIOB->CRH&= ~(GPIO_CRH_MODE14);
    GPIOB->CRH|= (GPIO_CRH_CNF14_0);
    GPIOB->CRH&= ~(GPIO_CRH_CNF14_1);

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
    uint8_t rbyte = 0;
    for (int i = 0; i < 8; i++)
    {
        if (byte & 0x80)
        {
            MOSI_HIGH();
        }
        else
        {
            MOSI_LOW();
        }
        SCK_HIGH();
        rbyte <<= 1;
        if (MISO_READ())
        {
            rbyte |= 0x01;
        }
        SCK_LOW();
        byte <<= 1;
    }
    return rbyte;
}

