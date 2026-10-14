#include "uart_rx.h"
#include "stdio.h"
#include "string.h"
#include "shell.h"

//定义缓冲区 
uint8_t RxBuff[BUFF_MAX_SIZE]={0};//dma搬运数据到这里
uint8_t RxLine[BUFF_MAX_SIZE]={0};//cpu处理这里的数据
volatile uint8_t RxReady=0;//判断是否接收数据完成
volatile uint8_t RxLen=0;//计算接收数据长度
volatile uint8_t RxErr=0;//dma接收出错标志

void parse_and_execute(char * rx_line)
{
    //清除末尾\r\n替换成\0->判断是否为NULL->清除前面空格和中间空格 最后比对
    rx_line[strcspn(rx_line,"\r\n")]='\0';
    char*name_cmd=strtok(rx_line," ");
    if(name_cmd==NULL)
    {
        printf("unknown cmd\r\n");
        return;
    }
    char*args=strtok(NULL," ");
    for (int i = 0; i < CMD_TABLE_SIZE; i++)
    {
        if(strcmp(name_cmd,cmd_table[i].name)==0)//命中
        {
            cmd_table[i].handler(args);
            return;
        }
    }
    printf("unknown cmd\r\n");
}
