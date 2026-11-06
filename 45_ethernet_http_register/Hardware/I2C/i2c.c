#include"i2c.h"
void I2C_Init(void)
{
    //1.打开GPIOB的时钟
    RCC->APB2ENR|=RCC_APB2ENR_IOPBEN;
    //2.PB6 PB7 通用开漏输出 mode 11  cnf 01
    GPIOB->CRL|=(GPIO_CRL_MODE7|GPIO_CRL_MODE6);
    GPIOB->CRL&=~(GPIO_CRL_CNF6_1|GPIO_CRL_CNF7_1);
    GPIOB->CRL|=(GPIO_CRL_CNF6_0|GPIO_CRL_CNF7_0);
}

void I2C_Start(void)
{
    //1.scl 和sda保持高
    SCL_HIGH;
    SDA_HIGH;
    I2C_DELAY;
    //2.scl 不变 sda 拉低
    SDA_LOW;
    I2C_DELAY;
}

void I2C_Stop(void)
{
    //1.先拉低SCL和SDA，确保起始状态正确
    SCL_LOW;
    SDA_LOW;
    I2C_DELAY;
    //2.拉高SCL（SDA保持低）
    SCL_HIGH;
    I2C_DELAY;
    //3.拉高SDA，产生停止信号
    SDA_HIGH;
    I2C_DELAY;
}

void I2C_ACK(void)
{
    //1.起始 SCL低 SDA高
    SCL_LOW;
    SDA_HIGH;
    I2C_DELAY;
    //2.SCL不动 SDA拉低保持
    SDA_LOW;
    I2C_DELAY;
    //3.SDA保持 SCL拉高采样
    SCL_HIGH;
    I2C_DELAY;
    //4.采样结束 SCL拉低
    SCL_LOW;
    I2C_DELAY;
    //5.SDA拉高 释放数据线
    SDA_HIGH;
    I2C_DELAY;
}

void I2C_NACK(void)
{
    //1.起始 SCL低 SDA高
    SCL_LOW;
    SDA_HIGH;
    I2C_DELAY;
    //2.SDA保持 SCL拉高采样
    SCL_HIGH;
    I2C_DELAY;
    //3.采样结束 SCL拉低
    SCL_LOW;
    I2C_DELAY;
}

uint8_t I2C_Wait4ACK(void)
{
    //1.SDA拉高 SCL拉低
    SDA_HIGH;
    SCL_LOW;
    I2C_DELAY;
    //2.SDA保持不变 SCL拉高
    SCL_HIGH;
    I2C_DELAY;
    //3.读数据
    uint16_t ack=READ_SDA;
    //4.采样结束 SCL拉低
    SCL_LOW;
    I2C_DELAY;

    return ack?NACK:ACK;
}

void I2C_SendByte(uint8_t byte)
{
    for(uint8_t i=0;i<8;i++)
    {
        //1.初始SCL SDA拉低
        SCL_LOW;
        SDA_LOW;
        I2C_DELAY;
        //2.数据有八位 I2C是高位先行，要先写最高位
        if(byte&0x80)//如果非0
        {
            SDA_HIGH;
        }
        else {SDA_LOW;}
        //3.SCL拉高采样
        SCL_HIGH;
        I2C_DELAY;
        //4.SCL拉低 采样结束
        SCL_LOW;
        I2C_DELAY;
        //5.左移byte
        byte<<=1;

    }
}

uint8_t I2C_ReadByte(void)
{
    uint8_t data=0;
    for(uint8_t i=0;i<8;i++)
    {
        //1.初始拉低SCL
        SCL_LOW;
        I2C_DELAY;
        //2.拉高SCL开始采样
        SCL_HIGH;
        I2C_DELAY;
        //3.把电平存放进data
        data<<=1;
        if(READ_SDA)
        {
            data|=0x01;
        }
        //4.拉低SCL 结束采样
        SCL_LOW;
        I2C_DELAY;
    }
    return data;
}

