/*======================================================================
 * 0.96" OLED Driver (SSD1306, 4-wire software I2C)
 * Ported to STM32 HAL. Original logic from JiangXieKeJi OLED library.
 *
 * I2C pins: SCL = PB8, SDA = PB9 (open-drain, software bit-bang)
 * Display: 128 x 64, page mode (8 pages of 8 rows)
 * Glyph data lives in OLED_Data.c / OLED_Data.h
 *====================================================================*/

#include "main.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* Framebuffer: 8 pages x 128 columns, each byte = 8 vertical pixels */
uint8_t OLED_DisplayBuf[8][128];

/*======================================================================
 * Low-level pin control (user MUST adapt to actual wiring)
 *====================================================================*/
void OLED_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)BitValue);
}

void OLED_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)BitValue);
}

void OLED_GPIO_Init(void)
{
    uint32_t i, j;

    /* power-on settle delay */
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < 1000; j++);
    }

    /* enable GPIOB clock, configure PB8/PB9 as open-drain output */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStructure.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* release bus */
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/*======================================================================
 * I2C software protocol
 *====================================================================*/
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA((uint8_t)(!!(Byte & (0x80 >> i))));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);   /* extra clock, no ACK processing */
    OLED_W_SCL(0);
}

/*======================================================================
 * Command / data write
 *====================================================================*/
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);   /* slave address */
    OLED_I2C_SendByte(0x00);   /* control byte: command */
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);   /* slave address */
    OLED_I2C_SendByte(0x40);   /* control byte: data */
    for (i = 0; i < Count; i++)
    {
        OLED_I2C_SendByte(Data[i]);
    }
    OLED_I2C_Stop();
}

/*======================================================================
 * Init / cursor
 *====================================================================*/
void OLED_Init(void)
{
    OLED_GPIO_Init();

    OLED_WriteCommand(0xAE);  /* display off */
    OLED_WriteCommand(0xD5);  /* clock divide ratio / oscillator freq */
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);  /* multiplex ratio */
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);  /* display offset */
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);  /* display start line */
    OLED_WriteCommand(0xA1);  /* segment remap (left/right) */
    OLED_WriteCommand(0xC8);  /* COM scan direction (up/down) */
    OLED_WriteCommand(0xDA);  /* COM pins hardware config */
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);  /* contrast */
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);  /* pre-charge period */
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);  /* VCOMH deselect level */
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);  /* entire display on/off */
    OLED_WriteCommand(0xA6);  /* normal/inverse display */
    OLED_WriteCommand(0x8D);  /* charge pump */
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);  /* display on */

    OLED_Clear();
    OLED_Update();
}

void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    /* NOTE: for 1.3" SH1106 (132 cols), uncomment "X += 2;" */
    /* X += 2; */

    OLED_WriteCommand(0xB0 | Page);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

/*======================================================================
 * Framebuffer helpers
 *====================================================================*/
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            OLED_DisplayBuf[j][i] = 0x00;
        }
    }
}

void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i < 128 && j >= 0 && j < 64)
            {
                OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
            }
        }
    }
}

void OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 128; i++)
        {
            OLED_DisplayBuf[j][i] = ~OLED_DisplayBuf[j][i];
        }
    }
}

void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i < 128 && j >= 0 && j < 64)
            {
                OLED_DisplayBuf[j / 8][i] ^= (0x01 << (j % 8));
            }
        }
    }
}

/*======================================================================
 * Point / line / shape drawing
 *====================================================================*/
void OLED_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X < 128 && Y >= 0 && Y < 64)
    {
        OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X < 128 && Y >= 0 && Y < 64)
    {
        if (OLED_DisplayBuf[Y / 8][X] & (0x01 << (Y % 8)))
        {
            return 1;
        }
    }
    return 0;
}

void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t steep;

    dx = X1 - X0;
    dy = Y1 - Y0;
    steep = (dx > 0) ? dx : -dx;
    steep = (steep < ((dy > 0) ? dy : -dy)) ? 1 : 0;

    if (steep)
    {
        temp = X0; X0 = Y0; Y0 = temp;
        temp = X1; X1 = Y1; Y1 = temp;
    }

    if (X0 > X1)
    {
        temp = X0; X0 = X1; X1 = temp;
        temp = Y0; Y0 = Y1; Y1 = temp;
    }

    dx = X1 - X0;
    dy = (Y1 > Y0) ? (Y1 - Y0) : (Y0 - Y1);
    d = 2 * dy - dx;
    incrE = 2 * dy;
    incrNE = 2 * (dy - dx);

    x = X0; y = Y0;
    if (steep) OLED_DrawPoint(y, x); else OLED_DrawPoint(x, y);
    while (x < X1)
    {
        if (d <= 0) d += incrE;
        else { d += incrNE; y += (Y1 > Y0) ? 1 : -1; }
        x++;
        if (steep) OLED_DrawPoint(y, x); else OLED_DrawPoint(x, y);
    }
}

void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    uint8_t i, j;
    if (IsFilled)
    {
        for (i = X; i < X + Width; i++)
        {
            for (j = Y; j < Y + Height; j++)
            {
                OLED_DrawPoint(i, j);
            }
        }
    }
    else
    {
        for (i = X; i < X + Width; i++) { OLED_DrawPoint(i, Y); OLED_DrawPoint(i, Y + Height - 1); }
        for (j = Y; j < Y + Height; j++) { OLED_DrawPoint(X, j); OLED_DrawPoint(X + Width - 1, j); }
    }
}

void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
    /* not implemented in original lib; kept as stub for API compatibility */
    (void)X0; (void)Y0; (void)X1; (void)Y1; (void)X2; (void)Y2; (void)IsFilled;
}

void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d, j;
    x = 0; y = Radius; d = 3 - 2 * Radius;
    while (x <= y)
    {
        if (IsFilled)
        {
            for (j = -x; j <= x; j++) { OLED_DrawPoint(X + j, Y + y); OLED_DrawPoint(X + j, Y - y); }
            for (j = -y; j <= y; j++) { OLED_DrawPoint(X + j, Y + x); OLED_DrawPoint(X + j, Y - x); }
        }
        else
        {
            OLED_DrawPoint(X + x, Y + y); OLED_DrawPoint(X - x, Y + y);
            OLED_DrawPoint(X + x, Y - y); OLED_DrawPoint(X - x, Y - y);
            OLED_DrawPoint(X + y, Y + x); OLED_DrawPoint(X - y, Y + x);
            OLED_DrawPoint(X + y, Y - x); OLED_DrawPoint(X - y, Y - x);
        }
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}

void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
    /* stub for API compatibility */
    (void)X; (void)Y; (void)A; (void)B; (void)IsFilled;
}

void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
    /* stub for API compatibility */
    (void)X; (void)Y; (void)Radius; (void)StartAngle; (void)EndAngle; (void)IsFilled;
}

/*======================================================================
 * Character / string / number display
 *====================================================================*/
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
    uint8_t i;
    uint8_t *Ptr;

    if (FontSize == OLED_8X16)
    {
        /* 8x16 glyph = 16 bytes: first 8 -> upper page, last 8 -> lower page */
        Ptr = (uint8_t *)&OLED_F8x16[((uint8_t)Char - ' ')];
        for (i = 0; i < 8; i++)
        {
            OLED_DisplayBuf[Y / 8][X + i]     = Ptr[i];
            OLED_DisplayBuf[Y / 8 + 1][X + i] = Ptr[i + 8];
        }
    }
    else
    {
        /* 6x8 glyph = 8 bytes, single page */
        Ptr = (uint8_t *)&OLED_F6x8[((uint8_t)Char - ' ')];
        for (i = 0; i < 8; i++)
        {
            OLED_DisplayBuf[Y / 8][X + i] = Ptr[i];
        }
    }
}

void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint8_t i, j;
    uint8_t Width = (FontSize == OLED_8X16) ? 8 : 6;

    for (i = 0; String[i] != '\0'; i++)
    {
        /* non-ASCII (UTF-8 multi-byte) -> try Chinese glyph table */
        if ((uint8_t)String[i] >= 0x80)
        {
            for (j = 0; ; j++)
            {
                /* end of table (empty Index) -> draw default box */
                if (OLED_CF16x16[j].Index[0] == '\0')
                {
                    OLED_ShowChar(X, Y, '?', OLED_8X16);
                    break;
                }
                /* compare the 3-byte UTF-8 code point */
                if (memcmp(&String[i], OLED_CF16x16[j].Index, 3) == 0)
                {
                    OLED_ShowChinese(X, Y, &OLED_CF16x16[j]);
                    break;
                }
            }
            /* skip remaining UTF-8 bytes (3-byte CJK here) */
            i += 2;
        }
        else
        {
            OLED_ShowChar(X, Y, String[i], FontSize);
        }
        X += Width;
    }
}

void OLED_ShowChinese(int16_t X, int16_t Y, const ChineseCell_t *Cell)
{
    uint8_t i;
    /* 16x16 glyph = 32 bytes: first 16 -> upper page, last 16 -> lower page */
    for (i = 0; i < 16; i++)
    {
        OLED_DisplayBuf[Y / 8][X + i]     = Cell->Data[i];
        OLED_DisplayBuf[Y / 8 + 1][X + i] = Cell->Data[i + 16];
    }
}

void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(X + i * ((FontSize == OLED_8X16) ? 8 : 6), Y,
                      (char)('0' + (Number / OLED_Pow(10, Length - i - 1) % 10)), FontSize);
    }
}

void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    if (Number < 0)
    {
        OLED_ShowChar(X, Y, '-', FontSize);
        Number = -Number;
    }
    else
    {
        OLED_ShowChar(X, Y, '+', FontSize);
    }
    OLED_ShowNum(X + ((FontSize == OLED_8X16) ? 8 : 6), Y, (uint32_t)Number, Length, FontSize);
}

void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint8_t SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = (uint8_t)(Number / OLED_Pow(16, Length - i - 1) % 16);
        if (SingleNumber < 10) SingleNumber += '0';
        else SingleNumber = (uint8_t)(SingleNumber - 10 + 'A');
        OLED_ShowChar(X + i * ((FontSize == OLED_8X16) ? 8 : 6), Y, (char)SingleNumber, FontSize);
    }
}

void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(X + i * ((FontSize == OLED_8X16) ? 8 : 6), Y,
                      (char)('0' + (Number / OLED_Pow(2, Length - i - 1) % 2)), FontSize);
    }
}

void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t PowNum;
    uint32_t IntNum;
    uint32_t FraNum;
    uint8_t W = (FontSize == OLED_8X16) ? 8 : 6;

    PowNum = OLED_Pow(10, FraLength);
    IntNum = (uint32_t)Number;
    FraNum = (uint32_t)((Number - (double)IntNum) * PowNum + 0.5);

    OLED_ShowNum(X, Y, IntNum, IntLength, FontSize);
    OLED_ShowChar(X + IntLength * W, Y, '.', FontSize);
    OLED_ShowNum(X + (IntLength + 1) * W, Y, FraNum, FraLength, FontSize);
}

void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i, j;
    for (j = 0; j < Height; j++)
    {
        for (i = 0; i < Width; i++)
        {
            if (X + i < 128 && Y + j < 64)
            {
                OLED_DisplayBuf[(Y + j) / 8][X + i] = Image[j * Width + i];
            }
        }
    }
}

void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
    char String[64];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    OLED_ShowString(X, Y, String, FontSize);
}

/*======================================================================
 * Refresh: push framebuffer to hardware
 *====================================================================*/
void OLED_Update(void)
{
    uint8_t j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
}

void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    uint8_t j;
    uint8_t StartPage = Y / 8;
    uint8_t EndPage = (Y + Height - 1) / 8;
    for (j = StartPage; j <= EndPage; j++)
    {
        OLED_SetCursor(j, X);
        OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
    }
}
