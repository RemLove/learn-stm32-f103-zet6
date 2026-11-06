#include "TCP.h"

void TCP_ServerStart(void)
{
    //1.获取端口号
   uint8_t status = getSn_SR(Sn);
   //2.判断端口状态 
   if(status==SOCK_CLOSED)
   {
    //如果是关闭状态，配置模式 打开端口
    int n = socket(Sn, Sn_MR_TCP, 8080,SF_TCP_NODELAY);
    if(n==Sn)//打开成功
    {
        printf("socket open success\r\n");
    }
   }
   else if(status==SOCK_INIT)
   {
    //启动监听
    int8_t result = listen(Sn);
    if(result==SOCK_OK)
    {
        printf("socket listen success\r\n");
    }
   }
   else if(status==SOCK_CLOSE_WAIT)
   {
    close(Sn);
   }
}

void TCP_SendData(uint8_t data[], uint16_t len)
{
     //1.获取端口号
   uint8_t status = getSn_SR(Sn);
    if(status==SOCK_ESTABLISHED)
    {
        send(Sn,data,len);
    }
}

void TCP_ReceiveData(uint8_t data[], uint16_t *len)
{
    //1.获取端口号
   uint8_t status = getSn_SR(Sn);
    if(status==SOCK_ESTABLISHED)
    {
       //如果接收到数据
       if(getSn_IR(Sn)&Sn_IR_RECV)
        {
            //清零标志位 写1
            setSn_IR(Sn,Sn_IR_RECV);
            //数据长度
            *len = getSn_RX_RSR(Sn);
            //读取数据
            recv(Sn,data,*len);
        }
    }
}
uint8_t ip[4]={192,168,172,183};
uint16_t port=8080;
void TCP_ClientStart(void)
{
    //1.获取端口号
   uint8_t status = getSn_SR(Sn);
   //2.判断端口状态 
   if(status==SOCK_CLOSED)
   {
    //如果是关闭状态，配置模式 打开端口
    int n = socket(Sn, Sn_MR_TCP, 8080,SF_TCP_NODELAY);
    if(n==Sn)//打开成功
    {
        printf("socket open success\r\n");
    }
   }
   else if(status==SOCK_INIT)
   {
    //启动连接
    int8_t result = connect(Sn, ip, port);
    if(result==SOCK_OK)
    {
        printf("socket connect success\r\n");
        TCP_SendData((uint8_t *)"Hello this is client ", strlen("Hello this is client "));
    }
   }
   else if(status==SOCK_CLOSE_WAIT)
   {
    close(Sn);
   }
}
