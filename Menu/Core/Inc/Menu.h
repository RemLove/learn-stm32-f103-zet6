#ifndef __MENU_H
#define __MENU_H
//菜单 设置  关于 计步 音量 亮度 

#include "main.h"
#include "OLED.h"
#include "OLED_Data.h"
//节点时文件夹还是叶子
typedef enum
{
    NODE_FOLDER = 0,
    NODE_LEAF
}NodeType;
//action函数指针类型
typedef void (*ActionFunc)(void);
//菜单节点结构体 id name type child编号 childrenNum action
typedef struct
{
    char name[20];
    NodeType type;
    int childId[10]; // 假设每个节点最多有10个子节点
    uint8_t childrenNum;
    ActionFunc action;

}Node_t;

void Menu_Init(void);

int here(void);

void Key_Up(void);

void Key_Down(void);

void Key_Enter(void);

void Key_Back(void);

void Menu_Display(void);
#endif /* __MENU_H */

