#include "lcd.h"
#include "fsmc.h"

#define SRAM_BANK1_4 0X6C000000
#define LCD_ADDR_CDM (uint16_t *)SRAM_BANK1_4
#define LCD_ADDR_DATA (uint16_t *)(SRAM_BANK1_4 + (1 << 11))

/* 画圆用 π。原先硬编码 3.13 是 π 的粗略近似（误差约 0.37%），
   会让圆首尾接不上一圈。这里给准确值。 */
#define LCD_PI 3.1415926

void LCD_Init(void)
{
    /* 注意：FSMC 已由 main.c 的 MX_FSMC_Init() 初始化完毕，此处不再重复初始化。
       原先这里调用的 FSMC_Init() 是 40 号寄存器工程的函数名，HAL 工程中不存在。 */
    LCD_Reset();     // 先复位
    LCD_RegConfig(); // 再写寄存器序列
}

void LCD_Reset(void)
{
    /* 复位时序：拉低 -> 等待 -> 拉高 -> 等待。
       注意不能只写 HAL_Delay 就完事，PG15 的低电平脉冲本身就是复位信号，
        gpio.c 里虽然把 PG15 初始置高了，但这里必须显式先拉低，否则屏不会复位。 */
    GPIOG->ODR &= ~GPIO_ODR_ODR15;
    HAL_Delay(100);
    GPIOG->ODR |= GPIO_ODR_ODR15;
    HAL_Delay(100);
}

void LCD_BGOn(void)
{
    GPIOB->ODR |= GPIO_ODR_ODR0;
}

void LCD_BGOff(void)
{
    GPIOB->ODR &= ~GPIO_ODR_ODR0;
}

void LCD_RegConfig(void)
{
    /* ILI9341 初始化序列 */

    /* 1. 软件复位 */
    LCD_WriteCmd(0x01);
    HAL_Delay(120);

    /* 2. 退出睡眠 */
    LCD_WriteCmd(0x11);
    HAL_Delay(120);

    /* 3. 电源控制 A */
    LCD_WriteCmd(0xCB);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x34);
    LCD_WriteData(0x02);

    /* 4. 电源控制 B */
    LCD_WriteCmd(0xCF);
    LCD_WriteData(0x00);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x30);

    /* 5. 驱动时序控制 A */
    LCD_WriteCmd(0xE8);
    LCD_WriteData(0x85);
    LCD_WriteData(0x00);
    LCD_WriteData(0x78);

    /* 6. 驱动时序控制 B */
    LCD_WriteCmd(0xEA);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);

    /* 7. 电源时序控制 */
    LCD_WriteCmd(0xED);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);

    /* 8. 泵比控制 */
    LCD_WriteCmd(0xF7);
    LCD_WriteData(0x20);

    /* 9. 像素格式：16bit/pixel */
    LCD_WriteCmd(0x3A);
    LCD_WriteData(0x55);

    /* 10. 帧率控制 */
    LCD_WriteCmd(0xB1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x18);

    /* 11. 显示功能控制 */
    LCD_WriteCmd(0xB6);
    LCD_WriteData(0x08);
    LCD_WriteData(0x82);
    LCD_WriteData(0x27);

    /* 12. 伽马校正 */
    LCD_WriteCmd(0xF2);
    LCD_WriteData(0x00);

    /* 13. 内存访问控制（方向）：竖屏 240×320，MX=1 水平翻转，bit3=1 用 BGR 顺序
       （这块屏物理是 BGR 排列，纯色/图片的红蓝分量都要按 BGR 送，否则红蓝互换） */
    LCD_WriteCmd(0x36);
    LCD_WriteData(0x48);

    /* 14. 伽马曲线 */
    LCD_WriteCmd(0xE0); /* 正伽马 */
    LCD_WriteData(0x0F);
    LCD_WriteData(0x31);
    LCD_WriteData(0x2B);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x08);
    LCD_WriteData(0x4E);
    LCD_WriteData(0xF1);
    LCD_WriteData(0x37);
    LCD_WriteData(0x07);
    LCD_WriteData(0x10);
    LCD_WriteData(0x03);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x09);
    LCD_WriteData(0x00);

    LCD_WriteCmd(0xE1); /* 负伽马 */
    LCD_WriteData(0x00);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x14);
    LCD_WriteData(0x03);
    LCD_WriteData(0x11);
    LCD_WriteData(0x07);
    LCD_WriteData(0x31);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x48);
    LCD_WriteData(0x08);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x31);
    LCD_WriteData(0x36);
    LCD_WriteData(0x0F);

    /* 15. 显示开 */
    LCD_WriteCmd(0x29);
    HAL_Delay(20);
}

void LCD_WriteCmd(uint16_t cmd)
{
    *LCD_ADDR_CDM = cmd;
}

void LCD_WriteData(uint16_t data)
{
    *LCD_ADDR_DATA = data;
}

uint16_t LCD_ReadData(void)
{

    return *LCD_ADDR_DATA;
}

uint32_t LCD_ReadId(void)
{
    uint32_t id = 0;

    /* 保持 40_fsmc__lcd_rigister 的原实现（那边同样调用本函数且显示正常），
       不额外加 dummy/延时——之前怀疑的"读命令未收尾"并非真因。 */
    LCD_WriteCmd(0xD3); /* ILI9341 Read ID4 命令 */

    LCD_ReadData();                       /* dummy read，丢弃 */
    id = (uint32_t)LCD_ReadData() << 24;  /* 制造商/参数 1 */
    id |= (uint32_t)LCD_ReadData() << 16; /* 参数 2 */
    id |= (uint32_t)LCD_ReadData() << 8;  /* 参数 3 */
    id |= (uint32_t)LCD_ReadData();       /* 参数 4 */

    return id;
}

void LCD_SetArea(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h)
{
    uint16_t x2 = x1 + w - 1;
    uint16_t y2 = y1 + h - 1;

    /* 边界钳位：ILI9341 的列/行地址寄存器越界后，Memory Write 的后续像素
       写入行为不可预测（花屏/撕裂）。画圆、画线等函数可能算出超屏坐标，
       在这里统一钳位，一处修好全局。 */
    if (x1 >= LCD_W) x1 = LCD_W - 1;
    if (y1 >= LCD_H) y1 = LCD_H - 1;
    if (x2 >= LCD_W) x2 = LCD_W - 1;
    if (y2 >= LCD_H) y2 = LCD_H - 1;

    LCD_WriteCmd(0x2A); /* 1. 列地址：起始列 → 结束列 */
    LCD_WriteData(x1 >> 8);
    LCD_WriteData(x1 & 0xFF);
    LCD_WriteData(x2 >> 8);
    LCD_WriteData(x2 & 0xFF);

    LCD_WriteCmd(0x2B); /* 2. 行地址：起始行 → 结束行 */
    LCD_WriteData(y1 >> 8);
    LCD_WriteData(y1 & 0xFF);
    LCD_WriteData(y2 >> 8);
    LCD_WriteData(y2 & 0xFF);

    LCD_WriteCmd(0x2C); /* 3. 准备写显存（Memory Write） */
}

void LCD_ClearAll(uint16_t color)
{
    /* 保持整屏一次连写（40_fsmc__lcd_rigister 已验证这样能正常显示）。
       之前改成"分行写"是多余的，只要 FSMC 时序对齐 40 号，长连写不会丢数据。 */
    uint32_t i;
    uint32_t total = (uint32_t)LCD_W * LCD_H;

    LCD_SetArea(0, 0, LCD_W, LCD_H); /* SetArea 末尾已发 0x2C */
    for (i = 0; i < total; i++)
    {
        LCD_WriteData(color);
    }
}

void LCD_WriteAsiciChar(uint16_t x, uint16_t y, uint16_t high, uint16_t fcolor, uint16_t bcolor, uint8_t c)
{
    // 1.先确定区域（SetArea 末尾已发 0x2C Memory Write，直接写数据即可）
    LCD_SetArea(x, y, high / 2, high);
    // 3.遍历区域，如果不是asic区域，就设置背景颜色  1608 1206
    uint16_t index = c - ' ';
    if (high == 16 || high == 12)
    {
        // 遍历每一行和每一列
        for (uint8_t i = 0; i < high; i++)
        {
            uint8_t tempByte = (high == 16) ? ascii_1608[index][i] : ascii_1206[index][i];
            for (uint8_t j = 0; j < high / 2; j++)
            {
                if (tempByte & 0x01)
                {
                    LCD_WriteData(fcolor);
                }
                else
                {
                    LCD_WriteData(bcolor);
                }
                tempByte >>= 1;
            }
        }
    }
    else if (high == 24) // 2412  两个字节表示一行
    {
        // 遍历每一行和每一列
        for (uint8_t i = 0; i < high * 2; i++)
        {
            uint16_t jCouynt = (i % 2) ? 4 : 8;
            uint8_t tempByte = ascii_2412[index][i];
            for (uint8_t j = 0; j < jCouynt; j++)
            {
                if (tempByte & 0x01)
                {
                    LCD_WriteData(fcolor);
                }
                else
                {
                    LCD_WriteData(bcolor);
                }
                tempByte >>= 1;
            }
        }
    }
    else if (high == 32) // 3216  两个字节表示一行
    {
        // 遍历每一行和每一列
        for (uint8_t i = 0; i < high * 2; i++)
        {
            uint16_t jCouynt = 8;
            uint8_t tempByte = ascii_3216[index][i];
            for (uint8_t j = 0; j < jCouynt; j++)
            {
                if (tempByte & 0x01)
                {
                    LCD_WriteData(fcolor);
                }
                else
                {
                    LCD_WriteData(bcolor);
                }
                tempByte >>= 1;
            }
        }
    }
}

void LCD_WriteAsiciStr(uint16_t x, uint16_t y, uint16_t high, uint16_t fcolor, uint16_t bcolor, uint8_t *str)
{
    uint8_t i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '\n') // 换行符：x 归 0，y 下移一行
        {
            x = 0;
            y += high;
        }
        else
        {
            LCD_WriteAsiciChar(x, y, high, fcolor, bcolor, str[i]);
            x += high / 2;
            if (x >= LCD_W) // 超出屏幕宽度则换行
            {
                x = 0;
                y += high;
            }
        }
        i++; // 移到下一个字符
    }
}

void LCD_WriteChinesChar(uint16_t x, uint16_t y, uint16_t high, uint16_t fcolor, uint16_t bcolor, uint8_t index)
{
    LCD_SetArea(x, y, high, high);

    for (uint8_t i = 0; i < 128; i++)
    {
        uint8_t tempByte = chinese[index][i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (tempByte & 0x01)   // 低位先画（LSB 在前，与 ASCII 字库一致）
            {
                LCD_WriteData(fcolor);
            }
            else
            {
                LCD_WriteData(bcolor);
            }
            tempByte >>= 1;
        }
    }
}

void LCD_WritePicture(uint16_t x, uint16_t y)
{
    LCD_SetArea(x, y, 240, 240);

    /* gImage_satella 是 240×240 的 RGB565 数据，每像素 2 字节。
       注意：Img2Lcd 生成的数组是低字节在前（小端），
       先取 [2*i] 作低字节、[2*i+1] 左移 8 作高字节。
       （高/低字节拼反的现象：轮廓正常但颜色乱成色块） */
    uint32_t pixel_cnt = sizeof(gImage_satella) / 2;

    for (uint32_t i = 0; i < pixel_cnt; i++)
    {
        uint16_t p = (uint16_t)(gImage_satella[2 * i] | (gImage_satella[2 * i + 1] << 8));
        LCD_WriteData(p);
    }
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    LCD_SetArea(x, y, w, w);
    /* 用 uint32_t：w*w 若在 uint16_t 域内运算，w>=256 会溢出回绕。 */
    uint32_t total = (uint32_t)w * w;
    for (uint32_t i = 0; i < total; i++)
    {
        LCD_WriteData(color);
    }
}

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t w, uint16_t color)
{
    /* 竖直线：斜率无穷大，单独处理 */
    if (x1 == x2)
    {
        if (y1 < y2)
            for (uint16_t y = y1; y <= y2; y++)
                LCD_DrawPoint(x1, y, w, color);
        else
            for (uint16_t y = y1; y >= y2; y--)
                LCD_DrawPoint(x1, y, w, color);
        return;
    }

    /* 水平线 */
    if (y1 == y2)
    {
        if (x1 < x2)
            for (uint16_t x = x1; x <= x2; x++)
                LCD_DrawPoint(x, y1, w, color);
        else
            for (uint16_t x = x1; x >= x2; x--)
                LCD_DrawPoint(x, y1, w, color);
        return;
    }

    /* 必须先转double 再相减。
       原写法 (y2-y1)/(x2-x1)*1.0 中，y2-y1 与 x2-x1 都先按 uint16_t
       做整数减法，当 x2<x1（或 y2<y1）时下溢成 60000 多，斜率直接算废。
       循环边界也改成 <= 且支持反向，保证端点一定画到。 */
    double k = ((double)y2 - (double)y1) / ((double)x2 - (double)x1);
    double b = (double)y1 - k * (double)x1;

    if (x1 < x2)
    {
        for (uint16_t x = x1; x <= x2; x++)
        {
            uint16_t y = (uint16_t)(k * x + b);
            LCD_DrawPoint(x, y, w, color);
        }
    }
    else
    {
        for (uint16_t x = x1; x >= x2; x--)
        {
            uint16_t y = (uint16_t)(k * x + b);
            LCD_DrawPoint(x, y, w, color);
        }
    }
}

void LCD_DrawCFX(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t w, uint16_t color)
{
    //上横
    LCD_DrawLine(x1,y1,x2,y1,w,color);
    //下横
    LCD_DrawLine(x1,y2,x2,y2,w,color);
    //左竖
    LCD_DrawLine(x1,y1,x1,y2,w,color);
    //右竖
    LCD_DrawLine(x2,y1,x2,y2,w,color);
}

void LCD_DrawCircle(uint16_t xCenter, uint16_t yCenter, uint16_t r, uint16_t w, uint16_t color)
{
    /* 用 LCD_PI 而非 3.13；cos/sin 返回 double，必须先转double 运算再显式
       截断为整数，否则隐式转换会产生截断误差，圆画不圆。 */
    for (uint16_t theta = 0; theta < 360; theta++)
    {
        double rad = LCD_PI * (double)theta / 180.0;
        uint16_t x = (uint16_t)((double)xCenter + (double)r * cos(rad));
        uint16_t y = (uint16_t)((double)yCenter + (double)r * sin(rad));
        LCD_DrawPoint(x, y, w, color);
    }
}

void LCD_DrawCircle_Pro(uint16_t xCenter, uint16_t yCenter, uint16_t r, uint16_t w, uint16_t bcolor, uint16_t fcolor)
{
    /* 实心圆：逐圈填充，越界由 LCD_SetArea 统一钳位处理 */
    for (uint16_t i = 0; i <= r; i++)
    {
        for (uint16_t theta = 0; theta < 360; theta++)
        {
            double rad = LCD_PI * (double)theta / 180.0;
            uint16_t x = (uint16_t)((double)xCenter + (double)i * cos(rad));
            uint16_t y = (uint16_t)((double)yCenter + (double)i * sin(rad));
            LCD_DrawPoint(x, y, w, bcolor);
        }
    }
}

