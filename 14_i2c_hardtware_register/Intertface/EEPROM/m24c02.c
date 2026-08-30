#include"m24c02.h"
void M24C02_Init(void)
{
    I2C_Init();
}

void M24C02_WriteByte(uint8_t innerAddr,uint8_t byte)
{
    //1.发送起始信号
    I2C_Start();
    //2.发送写地址
    I2C_SendAddr(W_ADDR);
    //4.发送内部地址
    I2C_SendByte(innerAddr);
    //6.发送数据
    I2C_SendByte(byte);
    //8.发送停止信号
    I2C_Stop();
    //等待写入完毕
    Delay_ms(5);
}
uint8_t M24C02_ReadByte(uint8_t innerAddr)//假写真读
{
    //1.发送起始信号
    I2C_Start();
    //2.发送写地址
    I2C_SendAddr(W_ADDR);
    //4.发送内部地址
    I2C_SendByte(innerAddr);
    //6.发出开始信号
    I2C_Start();
    //7.发送读地址
    I2C_SendAddr(R_ADDR);
    //发送非应答
    I2C_NACK();
    //11.发送停止信号
    I2C_Stop();
    //9.读取一个字节
    uint8_t ret=I2C_ReadByte();
    return ret;
}

void M24C02_WriteBytes(uint8_t innerAddr ,uint8_t*bytes,uint8_t size)
{
    //1.发送起始信号
    I2C_Start();
    //2.发送写地址
    I2C_SendAddr(W_ADDR);
    //4.发送内部地址
    I2C_SendByte(innerAddr);
    for(uint8_t i=0;i<size;i++)
    {
        //6.发送数据
        I2C_SendByte(bytes[i]);
    }
    //8.发送停止信号
    I2C_Stop();
    //等待写入完毕
    Delay_ms(5);

}
void M24C02_ReadBytes(uint8_t innerAddr ,uint8_t*buffer,uint8_t size)
{
    //1.发送起始信号
    I2C_Start();
    //2.发送写地址
    I2C_SendAddr(W_ADDR);
    //4.发送内部地址
    I2C_SendByte(innerAddr);
    //6.发出开始信号
    I2C_Start();
    //7.发送读地址
    I2C_SendAddr(R_ADDR);
    for(uint8_t i=0;i<size;i++)
    {
        //10.发送应答 最后一次发送非应答
        if(i==size-1)
        {
            I2C_NACK();
        }
        else
        {
            //11.发送停止信号
            I2C_Stop();
            I2C_ACK();
        }
        //9.读取一个字节
        buffer[i]=I2C_ReadByte();
    }

}
