#ifndef __ETH_H__
#define __ETH_H__

#include "stm32f10x.h"
#include "delay.h"
#include "spi.h"
#include "w5500.h"      // 内部会带上 wizchip_conf.h，提供 reg_wizchip_* / ctlnetwork / wiz_NetInfo

void ETH_Init(void);

#endif
