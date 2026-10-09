#include "UDP.h"

void UDP_Start(void)
{
    //1.获取端口号
   uint8_t status = getSn_SR(Sn);
   //2.判断端口状态 
   if(status==SOCK_CLOSED)
   {
    //如果是关闭状态，配置模式 打开端口
    int n = socket(Sn, Sn_MR_UDP, 9999,0);
    if(n==Sn)//打开成功
    {
        printf("socket open success\r\n");
    }
   }
   else if(status==SOCK_CLOSE_WAIT)
   {
    close(Sn);
   }
}

void UDP_SendData(uint8_t data[], uint16_t len, uint8_t dest_ip[4], uint16_t dest_port)
{
     //1.获取端口号
   uint8_t status = getSn_SR(Sn);
    if(status==SOCK_UDP)
    {
        sendto(Sn,data,len,dest_ip,dest_port);
    }
}

void UDP_ReceiveData(uint8_t data[], uint16_t *len, uint8_t* src_ip, uint16_t* src_port)
{
    //1.获取端口号
   uint8_t status = getSn_SR(Sn);
    if(status==SOCK_UDP)
    {
       //如果接收到数据
       if(getSn_IR(Sn)&Sn_IR_RECV)
        {
            //清零标志位 写1
            setSn_IR(Sn,Sn_IR_RECV);
            //数据长度
            *len = getSn_RX_RSR(Sn);
            //读取数据
            recvfrom(Sn,data,*len,src_ip,src_port);
        }
    }
}
