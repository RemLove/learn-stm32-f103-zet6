#include"i2c.h"
void I2C_Init(void)
{
    //1.打开GPIOB和I2C2的时钟
    RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
    RCC->APB1ENR|=RCC_APB1ENR_I2C1EN;
    //2.PB6 PB7 通用开漏输出 mode 11  cnf 01
    GPIOB->CRL|=(GPIO_CRL_MODE6|GPIO_CRL_MODE7);
    GPIOB->CRL&=~(GPIO_CRL_CNF6_0|GPIO_CRL_CNF7_0);
    GPIOB->CRL|=(GPIO_CRL_CNF6_1|GPIO_CRL_CNF7_1);
    //2.配置I2C
    //配置输入的时钟频率
    I2C1->CR2|=36;
    //配置CCR数据传输速率100kb/s SCL高电平时间为5us   1us是36个时钟周期
    I2C1->CCR|=180;
    //配置上升沿最大上升时间周期数+1
    I2C1->TRISE|=37;
    //使能I2C2模块
    I2C1->CR1|=I2C_CR1_PE;
}
uint8_t I2C_Start(void)
{
    I2C1->CR1|=I2C_CR1_START;
    //利用SB（statr bite）位判断是否发送成功 
    uint16_t timeout=65535;
    while((I2C1->SR1&I2C_SR1_SB)==0 && timeout )
    {
        timeout--;
    }
    return timeout ? OK:FAIL;
}

void I2C_Stop(void)
{
    I2C1->CR1|=I2C_CR1_STOP;
}

void I2C_ACK(void)
{
    I2C1->CR1|=I2C_CR1_ACK;
}

void I2C_NACK(void)
{
    I2C1->CR1&=~I2C_CR1_ACK;
}

uint8_t I2C_SendAddr(uint8_t addr)
{
    //向DR寄存器中写入地址
    I2C1->DR=addr;
    //等待应答
    uint16_t timeout=65535;
    while((I2C1->SR1&I2C_SR1_ADDR)==0 && timeout )
    {
        timeout--;
    }
    //访问sr2 清除ADDR
    I2C1->SR2;
    return timeout ? OK:FAIL;
}

uint8_t I2C_SendByte(uint8_t byte)
{
    //等待数据寄存器为空
    uint16_t timeout=65535;
    while((I2C1->SR1&I2C_SR1_TXE)==0 && timeout )
    {
        timeout--;
    }
    //把发送的数据填入数据寄存器
    I2C1->DR=byte;
    //等待ack应答
    timeout=65535;
    while((I2C1->SR1&I2C_SR1_BTF)==0 && timeout )
    {
        timeout--;
    }
    return timeout ? OK:FAIL;
}

uint8_t I2C_ReadByte(void)
{
    //等待数据寄存器非空
    uint16_t timeout=65535;
    while((I2C1->SR1&I2C_SR1_RXNE)==0 && timeout )
    {
        timeout--;
    }
    uint8_t ret=I2C1->DR;
    return timeout ?ret:FAIL;
}

