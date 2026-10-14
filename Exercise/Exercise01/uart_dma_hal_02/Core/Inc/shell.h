#ifndef __SHELL_H
#define __SHELL_H
#include <stdio.h>
#define CMD_TABLE_SIZE 4

//函数指针
typedef void(*cmd_handler_t)(char*args);
//命令 结构体 包含名字和实现(函数指针)
typedef struct 
{
    char*name;
    cmd_handler_t handler;
}cmd_table_t;

extern cmd_table_t cmd_table[CMD_TABLE_SIZE];

#endif

