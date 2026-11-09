#include "esp32.h"
#include "usart.h"


void ESP32_Init(void)
{
    uint8_t cmd[] = "AT+RST\r\n";
    HAL_UART_Transmit(&huart2, cmd, sizeof(cmd) - 1, HAL_MAX_DELAY);
    HAL_Delay(1000); // 等待 ESP32 重启完成
}

void ESP32_SendCmd(uint8_t * cmd, uint16_t len)
{
    HAL_UART_Transmit(&huart2, cmd, len, HAL_MAX_DELAY);
}

void ESP32_ReceiveCmd(uint8_t * cmd, uint16_t* len)
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart2, cmd, len);
}
