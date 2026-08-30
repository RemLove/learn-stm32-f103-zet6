#include "can.h"

void CAN_Init(void)
{
    //1.打开时钟 GPIOB AFIO CAN 
    RCC->APB2ENR|=RCC_APB2ENR_IOPBEN|RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR|=RCC_APB1ENR_CAN1EN;
    //2.复用CAN1
    AFIO->MAPR|=AFIO_MAPR_CAN_REMAP_1;
    AFIO->MAPR&=~AFIO_MAPR_CAN_REMAP;
    //3.配置工作模式 can1 pb8->rx pb9->tx
    GPIOB->CRH&=~(GPIO_CRH_MODE8|GPIO_CRH_CNF8|GPIO_CRH_MODE9|GPIO_CRH_CNF9);
    GPIOB->CRH|=GPIO_CRH_MODE9_1|GPIO_CRH_CNF9_1|GPIO_CRH_CNF8_0;
    //4.配置CAN1工作模式
    //4.1进入初始化  等待出现初始化标志位
    CAN1->MCR|=CAN_MCR_INRQ;
    while((CAN1->MSR&CAN_MSR_INAK)==0);
    //4.2退出睡眠模式 等待标志位
    while((CAN1->MSR&CAN_MSR_SLAK)!=0)
    {
        CAN1->MCR&=~CAN_MCR_SLEEP;
    }
    //4.3配置初始化模式 自动唤醒，自动离线
    CAN1->MCR|=CAN_MCR_AWUM|CAN_MCR_ABOM;
    //4.4配置位时序 环回静默模式 配置波特率 时钟上来36MHZ 分频35+1  第一个时间段强制1 TS2->2+1,TS2->5+1
    CAN1->BTR|=CAN_BTR_SILM|CAN_BTR_LBKM;
    CAN1->BTR&=~CAN_BTR_BRP;
    CAN1->BTR|=35<<0;
    CAN1->BTR&=~CAN_BTR_TS1;
    CAN1->BTR|=2<<16;
    CAN1->BTR&=~CAN_BTR_TS2;
    CAN1->BTR|=5<<20;
    //4.5退出初始化模式 等待标志位  
    CAN1->MCR&=~CAN_MCR_INRQ;
    while((CAN1->MSR&CAN_MSR_INAK)!=0);
}

void CAN_Send_Msg(uint32_t id, uint8_t *data, uint8_t len)
{
    //1.等待发送邮箱空闲
    while((CAN1->TSR&CAN_TSR_TME0)==0);
    //2.配置发送邮箱0
    CAN1->sTxMailBox[0].TIR|=id<<21;
    CAN1->sTxMailBox[0].TIR&=~CAN_TI0R_IDE;//标准帧
    CAN1->sTxMailBox[0].TIR&=~CAN_TI0R_RTR;//数据帧

    CAN1->sTxMailBox[0].TDTR &= ~(0x0000000FU);
    CAN1->sTxMailBox[0].TDTR |= len;

    //3.设置数据
    //3.1  清空数据寄存器
    CAN1->sTxMailBox[0].TDLR=0;
    CAN1->sTxMailBox[0].TDHR=0;
    //3.2存放数据
    for (int i = 0; i < len; i++)
    {
        if(i<4)
        {
            CAN1->sTxMailBox[0].TDLR|=data[i]<<(i*8);
        }
        else
        {
            CAN1->sTxMailBox[0].TDHR|=data[i]<<((i-4)*8);
        }
    }
    //4.发送请求
    CAN1->sTxMailBox[0].TIR|=CAN_TI0R_TXRQ;
    //5.等待发送完成
    while((CAN1->TSR&CAN_TSR_TXOK0)==0);
}
void CAN_Recive_Msg(RxMsg rxMsg[], uint8_t* MsgCount)
{
    //1.读取报文个数
    *MsgCount=(CAN1->RF0R&CAN_RF0R_FMP0);
    //2.把报文内容放在缓冲区
    for (uint8_t i = 0; i < *MsgCount; i++)
    {
        RxMsg*msg=&rxMsg[i];
        msg->id=CAN1->sFIFOMailBox[i].RIR>>21;
        msg->len=(CAN1->sFIFOMailBox[i].RDTR)&0x0f;
        uint32_t low=CAN1->sFIFOMailBox[i].RDLR;
        uint32_t high=CAN1->sFIFOMailBox[i].RDHR;
        for(uint8_t j=0;j<msg->len;j++)
        {
            if(j<4)
            {
                msg->data[j]=(low>>(8*j))&0xff;
            }
            else
            {
                msg->data[j]=(high>>(8*(j-4)))&0xff;
            }
        }
    }
    //3,释放输出邮箱
    CAN1->RF0R|=CAN_RF0R_RFOM0;
}

void CAN_FilterConfig(void)
{
    //1.进入初始化模式
    CAN1->FMR|=CAN_FMR_FINIT;
    //2.设置掩码模式
    CAN1->FM1R&=~CAN_FM1R_FBM0;
    //3.设置位宽
    CAN1->FS1R|=CAN_FS1R_FSC0;
    //4.设置关联
    CAN1->FFA1R&=~CAN_FFA1R_FFA0;
    //5. 设置过滤基准ID FR1
    CAN1->sFilterRegister[0].FR1 = 0x00000000;
    //6. 设置掩码 FR2，全0代表全部放行
    CAN1->sFilterRegister[0].FR2 = 0x00000000;
    //7. 激活过滤器0
    CAN1->FA1R |= CAN_FA1R_FACT0;
    //8. 退出初始化模式，过滤器开始工作
    CAN1->FMR &= ~CAN_FMR_FINIT;
}


