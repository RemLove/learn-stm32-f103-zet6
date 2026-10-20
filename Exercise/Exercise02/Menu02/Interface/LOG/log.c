#include "log.h"

#define LOG_DATA_START   0x00001000u
#define LOG_SECTOR_NUM   2u
#define LOG_DATA_END     (LOG_DATA_START + LOG_SECTOR_NUM * 4096u)
#define LOG_DATA_MAX     12   //数据最多十二位

/* RAM 副本：上电扫描得到，之后随写随更新 */
static uint16_t s_count;        /* 已记条数 */
static uint32_t s_write_addr;   /* 下一条写哪 */

//扫描法 元数据也放在数据区   初始化要读到上一次写到的地方 通过判断校验和(最后一位)和序列号(前两位)
void Log_Init(void)
{
    uint32_t addr=LOG_DATA_START;
    uint16_t count=0;
    while (addr<LOG_DATA_END)//从起始地址还是扫，通过校验和或者序列号不对来打断
    {
        uint8_t rec[LOG_REC_SIZE];
        uint8_t sum=0;
        Flash_ReadData(addr,rec,LOG_REC_SIZE);
        for (uint16_t i = 0; i < LOG_REC_SIZE-1; i++)
        {
            sum+=rec[i];
        }
        if(sum!=rec[LOG_REC_SIZE-1]) break;//校验和不对 打断循环
        uint16_t seq;//序列号
        seq=((uint16_t)rec[0]<<8)|rec[1];
        if(seq!=count) break;

        count++;
        addr+=LOG_REC_SIZE;
    }
    s_count=count;
    s_write_addr=addr;
}

void Log_Append(const uint8_t *data, uint8_t len)
{
    //先判断边界吧 数据和地址
    if (len>LOG_DATA_MAX) len=LOG_DATA_MAX;
    if(s_write_addr+LOG_REC_SIZE>LOG_DATA_END)//如果超出了地址
    {
        //全擦除再写
        for (uint8_t i = 0; i < LOG_SECTOR_NUM; i++)
        {
            Flash_SectorErase(LOG_DATA_START+4096*i);
        }
        s_count=0;
        s_write_addr=LOG_DATA_START;
    }   
    //十六字节分配
    uint8_t rec[LOG_REC_SIZE];
    //两位序列号 都是从零开始
    rec[0] = (uint8_t)(s_count >> 8);
    rec[1] = (uint8_t)s_count;
    //一位数据长度位
    rec[2]=len;

    for (uint8_t i = 0; i < LOG_DATA_MAX; i++)
    {
        rec[3 + i] = (i < len) ? data[i] : 0x00;
    }
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 15; i++) sum += rec[i]; rec[15] = sum;
    //数据位 需要判断是否跨页写
    if((s_write_addr&0xFF)+LOG_REC_SIZE>256u)
    {
        //跨页写           地址的低八位
        uint8_t first=(uint8_t)(256-(s_write_addr&0xFF));
        Flash_PageProgram(s_write_addr,rec,first);
        Flash_PageProgram((s_write_addr&0xFFFFFF00)+256,rec+first,(uint8_t)LOG_REC_SIZE-first);
    }
     else
    {
        Flash_PageProgram(s_write_addr, rec, LOG_REC_SIZE);
    }

    s_write_addr += LOG_REC_SIZE;
    s_count++;
}

uint16_t Log_Count(void)
{
    return s_count;
}

void Log_Read(uint16_t index, uint8_t *out16)
{
    Flash_ReadData(LOG_DATA_START + (uint32_t)index * LOG_REC_SIZE,
                   out16, LOG_REC_SIZE);
}
