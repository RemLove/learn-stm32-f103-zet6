#include "fsmc.h"


void FSMC_Init(void)
{
    //1.开启时钟 FSMC GPIOD E F G
    RCC->AHBENR|=RCC_AHBENR_FSMCEN;
    RCC->APB2ENR|=RCC_APB2ENR_IOPBEN|RCC_APB2ENR_IOPDEN|RCC_APB2ENR_IOPEEN
    |RCC_APB2ENR_IOPFEN|RCC_APB2ENR_IOPGEN;
    //2.初始化io引脚 
    FSMC_GPIO_Init();
  /* 3. fsmc的配置 Bank1的 4区 BCR4 */
    /* 3.1 存储块使能 */
    FSMC_Bank1->BTCR[4] |= FSMC_BCR4_MBKEN;
    /* 3.2 设置存储类型 00=SRAM ROM*/
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCR4_MTYP;
    /* 3.3 禁止闪存访问 */
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCR4_FACCEN;
    /* 3.4 地址和数据复用: 不复用 */
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCR4_MUXEN;
    /* 3.5 数据总线的宽度 16位宽度=01 */
    FSMC_Bank1->BTCR[4] &= ~FSMC_BCR4_MWID_1;
    FSMC_Bank1->BTCR[4] |= FSMC_BCR4_MWID_0;
    /* 3.6 写使能 */;
    FSMC_Bank1->BTCR[4] |= FSMC_BCR4_WREN;

    /* 4. fsmc的 时序 */
    /* 4.1 地址建立时间 对同步读写来说,永远一个周期 */
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTR4_ADDSET;
    /* 4.2 地址保持时间 对同步读写来说,永远一个周期 */
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTR4_ADDHLD;
    /* 4.3 数据保持时间 手册不能低于55ns 我们设置1us*/
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTR4_DATAST;
    FSMC_Bank1->BTCR[5] |= (71 << 8);
    /* 4.4 设置时序模式 */
    FSMC_Bank1->BTCR[5] &= ~FSMC_BTR4_ACCMOD;

}

void FSMC_GPIO_Init(void)
{
  //1.配置地址线 复用推挽输出 CNF 10  MODE 11
  //1.1 MODE 11
  GPIOF->CRL|=GPIO_CRL_MODE0|
  GPIO_CRL_MODE1|
  GPIO_CRL_MODE2|
  GPIO_CRL_MODE3|
  GPIO_CRL_MODE4|
  GPIO_CRL_MODE5;

  GPIOF->CRH|=GPIO_CRH_MODE12|
  GPIO_CRH_MODE13|
  GPIO_CRH_MODE14|
  GPIO_CRH_MODE15;

  GPIOG->CRL|=GPIO_CRL_MODE0|
  GPIO_CRL_MODE1|
  GPIO_CRL_MODE2|
  GPIO_CRL_MODE3|
  GPIO_CRL_MODE4|
  GPIO_CRL_MODE5;

  GPIOD->CRH|=GPIO_CRH_MODE12|
  GPIO_CRH_MODE13|
  GPIO_CRH_MODE11;

  //1.2 CNF 10
  GPIOF->CRL|=GPIO_CRL_CNF0_1|
  GPIO_CRL_CNF1_1|
  GPIO_CRL_CNF2_1|
  GPIO_CRL_CNF3_1|
  GPIO_CRL_CNF4_1|
  GPIO_CRL_CNF5_1;

  GPIOF->CRL&=~(GPIO_CRL_CNF0_0|
  GPIO_CRL_CNF1_0|
  GPIO_CRL_CNF2_0|
  GPIO_CRL_CNF3_0|
  GPIO_CRL_CNF4_0|
  GPIO_CRL_CNF5_0);

  GPIOF->CRH|=GPIO_CRH_CNF12_1|
  GPIO_CRH_CNF13_1|
  GPIO_CRH_CNF14_1|
  GPIO_CRH_CNF15_1;

  GPIOF->CRH&=~(GPIO_CRH_CNF12_0|
  GPIO_CRH_CNF13_0|
  GPIO_CRH_CNF14_0|
  GPIO_CRH_CNF15_0);

  GPIOG->CRL|=GPIO_CRL_CNF0_1|
  GPIO_CRL_CNF1_1|
  GPIO_CRL_CNF2_1|
  GPIO_CRL_CNF3_1|
  GPIO_CRL_CNF4_1|
  GPIO_CRL_CNF5_1;

  GPIOG->CRL&=~(GPIO_CRL_CNF0_0|
  GPIO_CRL_CNF1_0|
  GPIO_CRL_CNF2_0|
  GPIO_CRL_CNF3_0|
  GPIO_CRL_CNF4_0|
  GPIO_CRL_CNF5_0);

  GPIOD->CRH|=GPIO_CRH_CNF11_1|
  GPIO_CRH_CNF12_1|
  GPIO_CRH_CNF13_1;

  GPIOD->CRH&=~(GPIO_CRH_CNF11_0|
  GPIO_CRH_CNF12_0|
  GPIO_CRH_CNF13_0);

  //2.数据线
GPIOD->CRL |= (GPIO_CRL_MODE0 |
                GPIO_CRL_MODE1);
GPIOD->CRH |= (GPIO_CRH_MODE8 |
                GPIO_CRH_MODE9 |
                 GPIO_CRH_MODE10 |
                 GPIO_CRH_MODE14 |
                GPIO_CRH_MODE15);

GPIOE->CRL |= (GPIO_CRL_MODE7);
GPIOE->CRH |= (GPIO_CRH_MODE8 |
                GPIO_CRH_MODE9 |
                GPIO_CRH_MODE10 |
                GPIO_CRH_MODE11 |
                GPIO_CRH_MODE12 |
                GPIO_CRH_MODE13 |
                GPIO_CRH_MODE14 |
                GPIO_CRH_MODE15);

/* =============CNF=============== */
GPIOD->CRL |= (GPIO_CRL_CNF0_1 |
                GPIO_CRL_CNF1_1);
GPIOD->CRL &= ~(GPIO_CRL_CNF0_0 |
                GPIO_CRL_CNF1_0);

GPIOD->CRH |= (GPIO_CRH_CNF8_1 |
                GPIO_CRH_CNF9_1 |
                GPIO_CRH_CNF10_1 |
                GPIO_CRH_CNF14_1 |
                GPIO_CRH_CNF15_1);
GPIOD->CRH &= ~(GPIO_CRH_CNF8_0 |
                 GPIO_CRH_CNF9_0 |
                GPIO_CRH_CNF10_0 |
                GPIO_CRH_CNF14_0 |
                GPIO_CRH_CNF15_0);

GPIOE->CRL |= (GPIO_CRL_CNF7_1);
GPIOE->CRL &= ~(GPIO_CRL_CNF7_0);

GPIOE->CRH |= (GPIO_CRH_CNF8_1 |
                GPIO_CRH_CNF9_1 |
                GPIO_CRH_CNF10_1 |
                GPIO_CRH_CNF11_1 |
                GPIO_CRH_CNF12_1 |
                GPIO_CRH_CNF13_1 |
                GPIO_CRH_CNF14_1 |
                GPIO_CRH_CNF15_1);
GPIOE->CRH &= ~(GPIO_CRH_CNF8_0 |
                GPIO_CRH_CNF9_0 |
                GPIO_CRH_CNF10_0 |
                GPIO_CRH_CNF11_0 |
                GPIO_CRH_CNF12_0 |
                GPIO_CRH_CNF13_0 |
                GPIO_CRH_CNF14_0 |
                GPIO_CRH_CNF15_0);
 /* 3 其他控制端口  复用推挽输出 */
    /* 3.1 PD4: 读控制引脚  PD5： 写控制引脚 */
    GPIOD->CRL |= (GPIO_CRL_MODE4 |
                   GPIO_CRL_MODE5);
    GPIOD->CRL |= (GPIO_CRL_CNF4_1 |
                   GPIO_CRL_CNF5_1);
    GPIOD->CRL &= ~(GPIO_CRL_CNF4_0 |
                    GPIO_CRL_CNF5_0);

    /* 3.2  PG12：NE4*/
    GPIOG->CRH |= (GPIO_CRH_MODE12);
    GPIOG->CRH |= (GPIO_CRH_CNF12_1);
    GPIOG->CRH &= ~(GPIO_CRH_CNF12_0);

    /* 3.3  背光引脚PB0：通用推挽输出*/
    GPIOB->CRL |= GPIO_CRL_MODE0;
    GPIOB->CRL &= ~GPIO_CRL_CNF0;

    /* 3.4  重置引脚PG15：通用推挽输出*/
    GPIOG->CRH |= GPIO_CRH_MODE15;
    GPIOG->CRH &= ~GPIO_CRH_CNF15;

}


