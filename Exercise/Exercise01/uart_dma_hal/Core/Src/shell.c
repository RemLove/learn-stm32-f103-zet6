#include "shell.h"

static void led_cmd(char*args);
static void pwm_cmd(char*args);
static void info_cmd(char*args);
static void help_cmd(char*args);


//命令表 灯开关 pwm占空比 info返回固定信息 help打印命令表
static const cmd_table_t cmd_table[]=
{
  {"led_cmd",led_cmd},
  {"pwm_cmd",pwm_cmd},
  {"info_cmd",info_cmd},
  {"help_cmd",help_cmd}, 
};

//pe5
static void led_cmd(char*args)
{
    //1.判断是否为空指针 为空返回 
    if(args==NULL) return ;
    //2.比较是否是对应指令 on off
    if(strcmp(args,"on")==0)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);//写0点亮
        printf("led on\r\n");
    }
    else if(strcmp(args,"off")==0)
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
        printf("led off\r\n");
    }
    //3.如果没有对应指令 打印unknow cmd
    else
    {
        printf("unknow cmd\r\n");
    }
}
//pb5
static void pwm_cmd(char*args)
{

    //1.判断是否为NULL
    if(args==NULL) return;
    //2.args是字符串 需要转成int类型 用atoi库函数转 
    int duty=atoi(args);
    //3.防止越界
    if(duty>100) duty=100;
    if(duty<0) duty=0;
    //4.定时器3的通道2
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,duty);
}

static void info_cmd(char*args)
{
    (void)args;// 明确告诉编译器不使用该参数，防止警告
    printf("info_cmd\r\n");
}

static void help_cmd(char*args)
{
    (void)args;// 明确告诉编译器不使用该参数，防止警告
    printf("Available commands:\r\n");
    //循环变量命令表 
    for (uint8_t i = 0; i < 4; i++)
    {
        printf("  %-10s\r\n", cmd_table[i].name);
    }
}

void Parse_And_Execute(char * rx_line)
{
    //1.替换rx_line的\r\n为\0
    rx_line[strcspn(rx_line,"\r\n")]='\0';
    //2.清除rx_line开头空格 拿到第一段 判断是否第一段命令为NULL
    char*name_cmd=strtok(rx_line," ");
    if(name_cmd==NULL) return ;
    //3.切出参数，拿到第二段
    char*args=strtok(NULL," ");
    //4.判断是否命中
    for (uint8_t i = 0; i < 4; i++)
    {
        if(strcmp(name_cmd,cmd_table[i].name)==0)
        {
            cmd_table[i].handler(args);
            return;
        }
    }
    printf("unknow cmd\r\n");
}

