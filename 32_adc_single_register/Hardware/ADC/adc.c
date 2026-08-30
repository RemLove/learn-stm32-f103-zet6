#include "adc.h"

void ADC_Init(void)
{
    //1.时钟 寄存器版 ADC1 PC0
   RCC->APB2ENR|=RCC_APB2ENR_IOPCEN; //使能GPIOC时钟
    RCC->APB2ENR|=RCC_APB2ENR_ADC1EN; //使能ADC1时钟
    //2.工作模式
    //PC0模拟输入模式
    GPIOC->CRL&=~GPIO_CRL_MODE0; //输入模式
    GPIOC->CRL&=~GPIO_CRL_CNF0; //模拟输入模式
    //3.1 ADC工作模式 不扫描 连续转换
    ADC1->CR1&=~ADC_CR1_SCAN; //不扫描
    ADC1->CR2|=ADC_CR2_CONT; //连续转换
    //3. 2ADC时钟分频 12M
    ADC1->CR2|=ADC_CR2_ADON; //开启AD转换器
    //3.3 ADC采样时间 55.5周期
    ADC1->SMPR2|=ADC_SMPR2_SMP0_1|ADC_SMPR2_SMP0_0; //55.5周期
    //3.4右对齐
    ADC1->CR2&=~ADC_CR2_ALIGN; //右对齐
    //3.5 ADC通道选择 10号通道
    ADC1->SQR1&=~ADC_SQR1_L; //1个 
    //3.6序列
    ADC1->SQR3&=~ADC_SQR3_SQ1; //清空
    ADC1->SQR3|=ADC_SQR3_SQ1_1;
    //3.7选择软件触发模式
    ADC1->CR2|=ADC_CR2_EXTTRIG; //外部触发使能
    ADC1->CR2&=~ADC_CR2_EXTSEL; //软件触发 SW触发
}

void ADC_Start(void)
{
    //启动ADC转换
    ADC1->CR2|=ADC_CR2_ADON; //开启AD转换器
    ADC1->CR2|=ADC_CR2_SWSTART; //软件启动转换

}