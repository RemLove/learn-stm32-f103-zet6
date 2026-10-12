#include "esp32.h"

/* ============================================================================
 *  单点接收（一个 USART2，只有这里在读）
 * ----------------------------------------------------------------------------
 *  为什么必须这样改：
 *    STM32F1 的 USART 只有 1 级接收缓冲。原来的代码里
 *    STM32SendToESP32() 和 WIFI_TCP_ReadData() 各自开一个接收窗口去抢
 *    同一个 DR —— 谁正在等 OK，谁就把对方那条 +IPD 吃掉了；
 *    再加上 printf 到 USART1 的那 1~2ms 没人读 DR，后面的字节直接触发 ORE 丢失。
 *
 *    改成「中断驱动 + 环形缓冲」之后，硬件每收到一个字节立刻进中断搬进 ring，
 *    应用层永远不需要"专门腾出手来收"，也就不存在抢和丢。
 * ========================================================================== */

#define ESP32_RING_SIZE   2048u   /* 环形缓冲容量 */
#define ESP32_IT_CHUNK    64u     /* HAL 每块中断接收的落地大小 */

static uint8_t           ring[ESP32_RING_SIZE];
static volatile uint16_t ring_head = 0u;   /* 写入位置：只在中断里改 */
static volatile uint16_t ring_tail = 0u;   /* 读出位置：只在主流程里改 */
static uint8_t           it_chunk[ESP32_IT_CHUNK];

/* 兼容旧接口：最近一次命令响应的原文／长度 */
uint8_t  rxBuff[1024] = {0};
uint16_t rxSize = 0u;

/* ---------------------------------------------------------------------------
 * 中断回调：HAL 收满一块（64 字节）或检测到 IDLE 都会回调到这里
 * -------------------------------------------------------------------------*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
        for (uint16_t i = 0u; i < Size; i++)
        {
            uint16_t next = (uint16_t)((ring_head + 1u) % ESP32_RING_SIZE);
            if (next != ring_tail)              /* 没满才写，满了就丢弃新字节 */
            {
                ring[ring_head] = it_chunk[i];
                ring_head = next;
            }
            /* 满了也可以选择覆盖旧数据，但那会把帧结构截断，更难查，所以选丢新 */
        }

        /* 必须立刻重新挂上接收，否则只收得到第一块 —— 这一步最容易漏 */
        (void)HAL_UARTEx_ReceiveToIdle_IT(&huart2, it_chunk, ESP32_IT_CHUNK);
    }
}

/* ---------------------------------------------------------------------------
 * 出错回调：溢出(ORE)会让 HAL 中止接收并关掉中断，
 * 必须在这里重新挂上，否则整条接收链从此死掉（而且不报错，最难查）
 * -------------------------------------------------------------------------*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);        /* 读 SR 再读 DR，清掉溢出标志 */
        (void)HAL_UARTEx_ReceiveToIdle_IT(&huart2, it_chunk, ESP32_IT_CHUNK);
    }
}

uint16_t ESP32_RxAvail(void)
{
    return (uint16_t)((ring_head + ESP32_RING_SIZE - ring_tail) % ESP32_RING_SIZE);
}

void ESP32_RxFlush(void)
{
    ring_tail = ring_head;
}

/* 一次最多取 len 个字节，返回实际取到的个数；没数据就返回 0（不阻塞） */
uint16_t ESP32_RxRead(uint8_t *dst, uint16_t len)
{
    uint16_t n = 0u;

    while ((n < len) && (ring_tail != ring_head))
    {
        dst[n] = ring[ring_tail];
        n++;
        ring_tail = (uint16_t)((ring_tail + 1u) % ESP32_RING_SIZE);
    }
    return n;
}

/* 等关键字出现。注意：它会"消费"环形缓冲里直到关键字为止的字节 */
uint16_t ESP32_RxWait(const char *token, uint8_t *buff, uint16_t cap, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint16_t n = 0u;

    buff[0] = '\0';
    if (cap < 2u)
    {
        return 0u;
    }

    for (;;)
    {
        uint8_t c;

        while ((n < (uint16_t)(cap - 1u)) && (ESP32_RxRead(&c, 1u) == 1u))
        {
            buff[n] = (char)c;
            n++;
            buff[n] = '\0';
            if (strstr((char *)buff, token) != NULL)
            {
                return n;                       /* 命中关键字，立刻返回 */
            }
        }

        if ((HAL_GetTick() - start) >= timeout_ms)
        {
            return n;                           /* 超时：把收到的都带回去给调用者看 */
        }
    }
}

/* 发命令 + 指定超时 */
void STM32SendToESP32Ex(uint8_t *cmd, uint16_t size, uint32_t timeout_ms)
{
    /* 1.发命令 */
    if (HAL_UART_Transmit(&huart2, cmd, size, 1000u) != HAL_OK)
    {
        printf("TX fail\n");
        rxSize = 0u;
        return;
    }

    /* 2.等响应。不再"先收完再判 OK"，而是边收边判 */
    rxSize = ESP32_RxWait("OK", rxBuff, (uint16_t)sizeof(rxBuff), timeout_ms);

    if (rxSize != 0u)
    {
        printf("%.*s\n", (int)rxSize, (char *)rxBuff);
    }
    else
    {
        printf("no response\n");
    }
}

/* 发命令 + 默认超时 */
void STM32SendToESP32(uint8_t *cmd, uint16_t size)
{
    STM32SendToESP32Ex(cmd, size, ESP32_CMD_TIMEOUT_MS);
}

/* ---------------------------------------------------------------------------
 * 初始化
 * 注意：调用前 huart2 必须已经由 MX_USART2_UART_Init() 初始化过
 *      （USART2 的 NVIC 是在 HAL_UART_MspInit 里一并打开的）
 * -------------------------------------------------------------------------*/
void ESP32_Init(void)
{
    uint8_t *cmd;

    /* 1.先把中断接收挂起来，让环形缓冲开始工作 */
    ESP32_RxFlush();
    (void)HAL_UARTEx_ReceiveToIdle_IT(&huart2, it_chunk, ESP32_IT_CHUNK);

    /* 2.复位。AT+RST 会先回 OK，然后模块重启并吐 74880 波特的启动日志
          （波特率不同，在 115200 下收到的是乱码，忽略即可），所以超时给足 */
    cmd = (uint8_t *)"AT+RST\r\n";
    STM32SendToESP32Ex(cmd, (uint16_t)strlen((char *)cmd), 5000u);

    /* 3.用 AT 心跳探测它起没起来，比 HAL_Delay 盲等靠谱 */
    HAL_Delay(1500);
    for (uint8_t i = 0u; i < 10u; i++)
    {
        cmd = (uint8_t *)"AT\r\n";
        STM32SendToESP32Ex(cmd, 4u, 500u);
        if (strstr((char *)rxBuff, "OK") != NULL)
        {
            printf("ESP32 ready\n");
            break;
        }
    }
}
