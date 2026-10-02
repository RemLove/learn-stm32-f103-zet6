#include "main.h"   /* HAL header, replacing StdPeriph stm32f10x.h */

/**
  * @brief  Microsecond delay (DWT-based, does NOT occupy SysTick)
  * @note   cnt_per_us is derived from SystemCoreClock at runtime,
  *         so it works for both 36MHz and 72MHz with no coefficient change.
  * @param  xus  delay in microseconds, range: 0~4294967295
  * @retval None
  */
void Delay_us(uint32_t xus)
{
    uint32_t cnt_per_us = SystemCoreClock / 1000000;   /* core clocks per us */
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = xus * cnt_per_us;

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

/**
  * @brief  Millisecond delay
  * @param  xms  delay in milliseconds
  * @retval None
  */
void Delay_ms(uint32_t xms)
{
    while (xms--)
    {
        Delay_us(1000);
    }
}

/**
  * @brief  Second delay
  * @param  xs  delay in seconds
  * @retval None
  */
void Delay_s(uint32_t xs)
{
    while (xs--)
    {
        Delay_ms(1000);
    }
}
