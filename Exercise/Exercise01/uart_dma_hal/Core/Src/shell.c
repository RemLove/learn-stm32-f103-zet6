#include "shell.h"
//这四行全是shell.c的实现依赖 不是接口依赖 需要放到shell.c中
#include <stdio.h>//包含NULL
#include <string.h>//包含strcpy
#include <stdlib.h>//包含atoi
#include "tim.h"

static void led(char*args);
static void pwm(char*args);
static void info(char*args);
static void help(char*args);
//保存info要打印的led状态和pwm值(全局变量统一 g_ 前缀 + 小写下划线)
static const char *g_led_state = "off";
static int         g_pwm_state = 0;


//命令表 灯开关 pwm占空比 info返回固定信息 help打印命令表
static const cmd_entry_t cmd_table[]=
{
  {"led",led},
  {"pwm",pwm},
  {"info",info},
  {"help",help}, 
};

#define CMD_COUNT (sizeof(cmd_table) / sizeof(cmd_table[0]))//这个宏的实现依赖cmd_table 需要放在这个的下面

//pe5
static void led(char*args)
{
    //1.判断是否为空指针 为空返回 
    if(args==NULL) {printf("unknow cmd\r\n");return;}
    //2.比较是否是对应指令 on off
    if(strcmp(args,"on")==0)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);//写0点亮
        printf("led on\r\n");
        g_led_state="on";
    }
    else if(strcmp(args,"off")==0)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
        printf("led off\r\n");
        g_led_state="off";
    }
    //3.如果没有对应指令 打印unknow cmd
    else
    {
        printf("unknow cmd\r\n");
    }
}
//pb5
static void pwm(char*args)
{

    //1.判断是否为NULL
    if(args==NULL) {printf("unknow cmd\r\n");return;}
    //2.args是字符串 需要转成int类型 用atoi库函数转 
    int duty=atoi(args);
    //3.防止越界
    if(duty>100) duty=100;
    if(duty<0) duty=0;
    //4.定时器3的通道2
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,duty);
    printf("%d\r\n",duty);
    g_pwm_state=duty;
}

static void info(char*args)
{
    (void)args;// 明确告诉编译器不使用该参数，防止警告
    printf("info\r\n");
    printf("led_state : %s\r\n",g_led_state);
    printf("pwm_state : %d\r\n",g_pwm_state);
}

static void help(char*args)
{
    (void)args;// 明确告诉编译器不使用该参数，防止警告
    printf("Available commands:\r\n");
    //循环变量命令表 (条数用 CMD_COUNT, 增删命令不用改这里)
    for (uint8_t i = 0; i < CMD_COUNT; i++)
    {
        printf("  %-10s\r\n", cmd_table[i].name);
    }
}

void parse_and_execute(char *rx_line)
{
    //1.替换rx_line的\r\n为\0
    rx_line[strcspn(rx_line,"\r\n")]='\0';
    //2.清除rx_line开头空格 拿到第一段 判断是否第一段命令为NULL
    char*name_cmd=strtok(rx_line," ");
    if(name_cmd==NULL) return ;
    //3.切出参数，拿到第二段
    char*args=strtok(NULL," ");
    //4.判断是否命中
    for (uint8_t i = 0; i < CMD_COUNT; i++)
    {
        if(strcmp(name_cmd,cmd_table[i].name)==0)
        {
            cmd_table[i].handler(args);
            return;
        }
    }
    printf("unknow cmd\r\n");
}

