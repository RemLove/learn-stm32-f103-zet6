#ifndef __MENU_H
#define __MENU_H

#include "lcd.h"
#include "key.h"
#include <stdio.h>

/* 菜单初始化：清屏并画出主菜单，main() 里调一次 */
void Menu_Init(void);

/* 菜单扫描：主循环里反复调，负责响应按键和定时刷新 */
void Menu_Scan(void);

#endif
