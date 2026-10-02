#include "eth.h"

// 网络参数（按你的 192.168.44.x 网段）
// MAC 首字节必须为偶数（组播位为 0），这里 0x00 合法
uint8_t g_mac[6] = {0x00, 0x08, 0xDC, 0x11, 0x22, 0x33};
uint8_t g_ip[4]  = {192, 168, 44, 222};
uint8_t g_gw[4]  = {192, 168, 44, 1};
uint8_t g_sn[4]  = {255, 255, 255, 0};

// socket 缓冲区分配：W5500 共 16KB 收发，按 KB 给每个 socket 分
// 8 个 socket 各 2KB 收发，Tx/Rx 各 2KB，合计 16KB 刚好用满
static uint8_t tx_size[8] = {2, 2, 2, 2, 2, 2, 2, 2};
static uint8_t rx_size[8] = {2, 2, 2, 2, 2, 2, 2, 2};

// W5500 复位脚 PG7（低电平复位，拉高后延时释放）
static void w5500_hw_reset(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPGEN;   // 开 GPIOG 时钟

    // PG7 属于低 8 位端口，配 CRL。先清掉 PG7 的 4 个配置位(bit28~31)
    GPIOG->CRL &= ~(0xFu << 28);
    // MODE7 = 11（50MHz 通用推挽输出），CNF7 = 00
    GPIOG->CRL |= (0x3u << 28);

    // 复位：拉低 > 500us，再拉高
    GPIOG->ODR &= ~GPIO_ODR_ODR7;
    Delay_ms(10);
    GPIOG->ODR |=  GPIO_ODR_ODR7;
    // W5500 复位释放后需等 PHY 稳定，留足时间再访问
    Delay_ms(10);
}

// 绑定 W5500 库所需的 SPI / 片选回调，复用 spi.c 里的实现
static void w5500_reg_callbacks(void)
{
    reg_wizchip_cs_cbfunc(SPI_Start, SPI_Stop);        // 片选 CS_LOW / CS_HIGH
    reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte); // 读 / 写字节回调
}

void ETH_Init(void)
{
    // 1. 初始化 SPI2（PB12 CS / PB13 SCK / PB14 MISO / PB15 MOSI，模式 0）
    SPI_Init();

    // 2. 绑定底层回调（必须在访问 W5500 之前）
    w5500_reg_callbacks();

    // 3. 硬件复位 W5500
    w5500_hw_reset();

    // 4. 初始化芯片（分配 socket 缓冲区），必须先于任何寄存器配置
    wizchip_init(tx_size, rx_size);

    // 5. 一次性下发网络参数
    wiz_NetInfo netinfo = {
        .mac = {g_mac[0], g_mac[1], g_mac[2], g_mac[3], g_mac[4], g_mac[5]},
        .ip  = {g_ip[0], g_ip[1], g_ip[2], g_ip[3]},
        .sn  = {g_sn[0], g_sn[1], g_sn[2], g_sn[3]},
        .gw  = {g_gw[0], g_gw[1], g_gw[2], g_gw[3]},
        .dns = {0, 0, 0, 0},
        .dhcp = NETINFO_STATIC,
    };
    ctlnetwork(CN_SET_NETINFO, (void*)&netinfo);
}
