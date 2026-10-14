#include "shell.h"
#include <string.h>
#include "gpio.h"
#include <stdio.h>
#include "tim.h"
#include <stdlib.h>

static void led(char*args);
static void pwm(char*args);
static void info(char*args);
static void help(char*args);

//两个全局变量 用于info展示led pwm信息
static char* g_led_state="off";
static int g_pwm_state=0;

cmd_table_t cmd_table[CMD_TABLE_SIZE]=
{
    "led",led,
    "pwm",pwm,
    "info",info,
    "help",help,
};
//每个命令具体实现
static void led(char*args)
{
    if(args==NULL)
    {
        printf("unknown cmd\r\n");
        return;
    }
    if(strcmp(args,"on")==0)
    {
        HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_RESET);//写0点亮
        g_led_state="on";
    }
    else if(strcmp(args,"off")==0)
    {
        HAL_GPIO_WritePin(LED0_GPIO_Port,LED0_Pin,GPIO_PIN_SET);//写0点灭
         g_led_state="off";
    }
    else 
    {
       printf("unknown cmd\r\n");
    }
}

static void pwm(char*args)
{
    if(args==NULL)
    {
        printf("unknown cmd\r\n");
        return;
    }
    int duty=atoi(args);//将字符串args转换成int类型
    //两次范围检查
    if(duty>=100) duty=99;
    if(duty<0) duty=0;
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,duty);
    g_pwm_state=duty;
    printf("duty:%d\r\n",duty);
}
static void info(char*args)
{
    (void)args;//告诉编译器不需要这个
    printf("g_led_state:%s\r\n",g_led_state);
    printf("g_pwm_state:%d\r\n",g_pwm_state);
}
static void help(char*args)
{
    (void)args;//告诉编译器不需要这个
    //循环变量命令表 (条数用 CMD_COUNT, 增删命令不用改这里)
    for (uint8_t i = 0; i < CMD_TABLE_SIZE; i++)
    {
        printf("  %-10s\r\n", cmd_table[i].name);
    }
}
