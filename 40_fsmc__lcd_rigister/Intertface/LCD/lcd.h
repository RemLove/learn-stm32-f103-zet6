#ifndef __LCD_H
#define __LCD_H

#define LCD_W 240
#define LCD_H 320

/* 常见颜色 */
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 // 棕色
#define BRRED 0XFC07 // 棕红色
#define GRAY 0X8430  // 灰色



#include "stm32f10x.h"
#include "delay.h"
#include "fsmc.h"
#include "LCD_Front.h"
#include <math.h>

void LCD_Init(void);

void LCD_Reset(void);

void LCD_BGOn(void);

void LCD_BGOff(void);

void LCD_RegConfig(void);

void LCD_WriteCmd(uint16_t cmd);

void LCD_WriteData(uint16_t data);

uint16_t LCD_ReadData(void);

uint32_t LCD_ReadId(void);

void LCD_SetArea(uint16_t x1,uint16_t y1,uint16_t w,uint16_t h);

void LCD_ClearAll(uint16_t color);

void LCD_WriteAsiciChar(uint16_t x,uint16_t y,uint16_t high,uint16_t fcolor,uint16_t bcolor,uint8_t c);

void LCD_WriteAsiciStr(uint16_t x, uint16_t y, uint16_t high, uint16_t fcolor, uint16_t bcolor, uint8_t* str);

void LCD_WriteChinesChar(uint16_t x,uint16_t y,uint16_t high,uint16_t fcolor,uint16_t bcolor,uint8_t index);

void LCD_WritePicture(uint16_t x,uint16_t y);

void LCD_DrowPoint(uint16_t x,uint16_t y,uint16_t w,uint16_t color);

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t w, uint16_t color);
//长方形
void LCD_DrawCFX(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t w, uint16_t color);
//画圆
void LCD_DrawCircle(uint16_t xCenter,uint16_t yCenter ,uint16_t r,uint16_t w , uint16_t color);
//实心圆
void LCD_DrawCircle_Pro(uint16_t xCenter,uint16_t yCenter ,uint16_t r,uint16_t w , uint16_t color,uint16_t fcolor);

#endif
