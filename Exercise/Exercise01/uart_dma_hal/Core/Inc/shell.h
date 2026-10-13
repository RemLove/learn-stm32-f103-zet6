#ifndef __SHELL_H
#define __SHELL_H

#include <stdio.h>//包含NULL
#include <string.h>//包含strcpy
#include <stdlib.h>//包含atoi
#include "tim.h"
//1.函数指针  先是一个指针指向void -- (char*args)的函数 然后用typedef起别名了 叫cmd_handler_t 这是一个指针
typedef void(*cmd_handler_t)(char*args);
//2.命令结构体 包含名字 和 实现(函数指针)
typedef struct 
{
    char*name;
    cmd_handler_t handler;//指针
}cmd_table_t;

void Parse_And_Execute(char *rx_line);

#endif 

