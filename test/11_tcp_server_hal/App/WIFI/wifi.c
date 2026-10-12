#include "wifi.h"

//定义两个内部函数 设置ESP32 WIFI模式
static void WIFI_SET_STA(void);
static void WIFI_SET_AP(void);

/* ============================================================================
 *  累积缓冲
 * ----------------------------------------------------------------------------
 *  环形缓冲里的字节是零散到达的，一帧 "+IPD,0,5,"ip",8080:hello" 可能被拆成
 *  好几次中断才收全。所以这里先把字节攒起来，等整帧到齐了再解析、再消费掉。
 *  没到齐就一个字都不动，下一轮接着攒 —— 这就是"按帧长收"而不是"按时间收"。
 * ========================================================================== */
#define WIFI_ACC_SIZE 1536u
static uint8_t  acc[WIFI_ACC_SIZE + 1u];    /* +1 留给字符串结束符 '\0' */
static uint16_t accLen = 0u;

/* 丢掉 acc 开头的 eaten 个字节 */
static void ACC_Drop(uint16_t eaten)
{
    if (eaten >= accLen)
    {
        accLen = 0u;
    }
    else
    {
        memmove(acc, &acc[eaten], (size_t)(accLen - eaten));
        accLen = (uint16_t)(accLen - eaten);
    }
    acc[accLen] = '\0';
}

void WIFI_Init(WIFI_MODE mode)
{
    //1.初始化esp32
    ESP32_Init();
    //2.设置模式
    if(mode==STA)
    {
        WIFI_SET_STA();
    }
    else if(mode==AP)
    {
        WIFI_SET_AP();
    }
}

static void WIFI_SET_STA(void)
{
    //1.设置模式
    uint8_t* cmd="AT+CWMODE=1\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //2.连接路由器 —— 实际要 3~6 秒，必须给长超时，否则必然误报 no response
    cmd="AT+CWJAP=\"越王\",\"113937604\"\r\n";
    STM32SendToESP32Ex(cmd,strlen((char*)cmd),ESP32_LONG_TIMEOUT_MS);
    //3.查询本机拿到的 IP。STA 模式查的是 CIPSTA，不是 CIPAP（那是 AP 自己的 IP）
    cmd="AT+CIPSTA?\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
}

static void WIFI_SET_AP(void)
{
    //1.设置模式
    uint8_t* cmd="AT+CWMODE=2\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //2.设置ip
    cmd="AT+CWSAP=\"Satella\",\"113937604\",5,3\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //3.设置ip
    cmd="AT+CIPAP=\"192.168.8.1\"\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
}
void WIFI_TCP_ServertStart(void)
{
    //1.设置多连接模式
    uint8_t* cmd="AT+CIPMUX=1\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //2.让 +IPD 带上远端的 IP 和端口（需要 ESP-AT v1.2 以上，老固件会回 ERROR）
    cmd="AT+CIPDINFO=1\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //3.接收模式 0 = 主动模式（也是默认值）：收到数据立刻用 +IPD 推给 MCU
    //  ★ 这条只设一次就够了，所以放在初始化里。
    //    以前写在主循环的 WIFI_TCP_ReadData() 里，每轮都发一次 ——
    //    每次发都开一个接收窗口，正好把期间到达的 +IPD 吃掉，是随机丢包的元凶。
    cmd="AT+CIPRECVMODE=0\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
    //4.开启 TCP server，端口 8080
    cmd="AT+CIPSERVER=1,8080\r\n";
    STM32SendToESP32(cmd,strlen((char*)cmd));
}

void WIFI_TCP_SendData(uint8_t id,uint8_t* data,uint16_t size)
{
    uint8_t  cmd[32];
    uint16_t n;

    //1.发 CIPSEND 命令，告诉 ESP32 往哪个链路发多少字节
    sprintf((char*)cmd,"AT+CIPSEND=%d,%d\r\n",id,size);
    if(HAL_UART_Transmit(&huart2,cmd,(uint16_t)strlen((char*)cmd),1000)!=HAL_OK)
    {
        return;
    }
    //2.必须等到 '>' 提示符出现才能灌数据。
    //  直接发会有竞态：ESP32 还没准备好，数据就被当成 AT 命令吃掉了。
    n = ESP32_RxWait(">",rxBuff,(uint16_t)sizeof(rxBuff),2000u);
    if((n==0u)||(strchr((char*)rxBuff,'>')==NULL))
    {
        //多半是 ERROR：链路不存在 / 对端已断开
        printf("CIPSEND no prompt: %u\n",(unsigned)n);
        return;
    }
    //3.灌数据
    HAL_UART_Transmit(&huart2,data,size,1000);
    //4.把 SEND OK 收掉，免得它留在缓冲里干扰下一帧解析
    (void)ESP32_RxWait("SEND OK",rxBuff,(uint16_t)sizeof(rxBuff),2000u);
}

void WIFI_TCP_ReadData(uint8_t* buff,uint16_t cap,uint16_t* size,
                       uint8_t* id,char* ip,uint16_t* port)
{
    uint8_t* pIPD;
    uint8_t* colon;
    uint16_t headLen;
    uint16_t eaten;
    int      n;

    *size = 0u;

    //1.把环形缓冲里已经到的字节全部搬进累积缓冲
    //  不阻塞、不丢字节：有多少搬多少，没有就立刻返回
    if(accLen < WIFI_ACC_SIZE)
    {
        accLen += ESP32_RxRead(&acc[accLen],(uint16_t)(WIFI_ACC_SIZE - accLen));
        acc[accLen] = '\0';
    }

    //2.找帧头 "+IPD,"
    pIPD = (uint8_t*)strstr((char*)acc,"+IPD,");
    if(pIPD == NULL)
    {
        //整块里都没有帧头：只保留末尾 5 字节（可能是个还没拼完的 "+IPD,"），
        //其余都是垃圾字节（AT 回显、"SEND OK"、"0,CONNECT" 等上报），丢掉
        if(accLen > 5u)
        {
            ACC_Drop((uint16_t)(accLen - 5u));
        }
        return;
    }
    //3.帧头之前的全是杂字节，丢掉，让 acc 从 "+IPD," 开始
    if(pIPD != acc)
    {
        ACC_Drop((uint16_t)(pIPD - acc));
    }

    //4.解析帧头：+IPD,<id>,<len>,"<ip>",<port>:
    //  ★ 指针直接传（它们已经是指针了），不要写 &id 或 &port —— 那是 uint8_t**
    //  ★ %[^\"] 必须带宽度 15，否则 ip 只有 16 字节会被写穿
    *id = 0u;
    ip[0] = '\0';
    *port = 0u;
    n = sscanf((char*)acc,"+IPD,%hhu,%hu,\"%15[^\"]\",%hu",id,size,ip,port);
    if(n < 2)
    {
        // 连 id 和 len 都没解析出来，说明帧头还没收全，下一轮再来
        return;
    }

    //5.找数据分隔符 ':'。它前面的整个头长度就是 (冒号位置 - acc) + 1
    colon = (uint8_t*)strchr((char*)acc,':');
    if(colon == NULL)
    {
        return;                             //分隔符还没到
    }
    headLen = (uint16_t)(colon - acc) + 1u;

    //6.帧长超过累积缓冲能装下的 → 一定是解析错了（或对端发了超长帧）
    //  丢掉帧头重新同步，否则会在这里永久卡住
    if(((uint32_t)headLen + (uint32_t)(*size)) > (uint32_t)WIFI_ACC_SIZE)
    {
        ACC_Drop(5u);
        *size = 0u;
        return;
    }
    //7.数据还没到齐 → 一个字都不动，等下一轮接着攒
    if(accLen < (uint16_t)(headLen + *size))
    {
        return;
    }
    //8.长度保护：超出调用方缓冲容量就整帧丢弃（但要从缓冲里消费掉，避免卡死）
    if((*size == 0u) || (*size > cap))
    {
        eaten = (uint16_t)(headLen + *size);
        ACC_Drop(eaten);
        *size = 0u;
        return;
    }

    //9.拷走数据，并把这一帧从累积缓冲里消费掉
    memcpy(buff,colon + 1u,*size);
    eaten = (uint16_t)(headLen + *size);
    ACC_Drop(eaten);
}
