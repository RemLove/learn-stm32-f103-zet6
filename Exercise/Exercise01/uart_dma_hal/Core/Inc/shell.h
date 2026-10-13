#ifndef __SHELL_H
#define __SHELL_H

//1.函数指针  先是一个指针指向void -- (char*args)的函数 然后用typedef起别名了 叫cmd_handler_t 这是一个指针
typedef void(*cmd_handler_t)(char *args);
//2.命令结构体 包含名字 和 实现(函数指针) —— 它描述的是"一条命令", 所以叫 cmd_entry_t
typedef struct
{
    char *name;
    cmd_handler_t handler;//指针
} cmd_entry_t;

//3.对外唯一接口: 命名统一为小写下划线(snake_case)
void parse_and_execute(char *rx_line);

#endif 

