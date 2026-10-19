#include "log.h"
#include "flash.h"
//宏定义 魔数 元数据地址 记录区起始地址 用扇区数量 记录区结束地址 条数(方便回绕)

#define LOG_MAGIC        0xA55A5AA5u//魔数
#define LOG_META_ADDR    0x00000000u//元数据放扇区0 一般存指针计数啥的 需要单独擦写  把扇区擦除磨损隔离开
#define LOG_DATA_START   0x00001000u//记录区起始地址
#define LOG_SECTOR_NUM   2u         //扇区数量
#define LOG_DATA_END     (LOG_DATA_START + LOG_SECTOR_NUM * 4096u)//记录区结束地址 写指针到这就得回头
/* 总容量 = 8192 / 16 = 512 条。写满了就擦掉重来 */
#define LOG_MAX_COUNT    ((LOG_DATA_END - LOG_DATA_START) / LOG_REC_SIZE)
//ram里面的副本
static uint16_t s_count;        /* 已记条数 */
static uint32_t s_write_addr;   /* 下一条写哪 */

//内部函数 把ram里面的状态同步回flash
static void Log_SaveMeta(void)
{
    //元数据扇区布局 12字节 相当于就是把元数据里面的数据打包成一个数组方便擦和写
    //0-3魔数 4-5条数 6-7保留0 8-10写指针(三个字节 二十四位 flash就是24位够了) 11保留0
    uint8_t meta[12];
    meta[0] = (uint8_t)(LOG_MAGIC >> 24);
    meta[1] = (uint8_t)(LOG_MAGIC >> 16);
    meta[2] = (uint8_t)(LOG_MAGIC >> 8);
    meta[3] = (uint8_t)LOG_MAGIC;

    meta[4] = (uint8_t)(s_count >> 8);
    meta[5] = (uint8_t)s_count;
    meta[6] = 0x00;
    meta[7] = 0x00;

    meta[8]  = (uint8_t)(s_write_addr >> 16);
    meta[9]  = (uint8_t)(s_write_addr >> 8);
    meta[10] = (uint8_t)s_write_addr;
    meta[11] = 0x00;
    Flash_SectorErase(LOG_META_ADDR);
    Flash_PageProgram(LOG_META_ADDR, meta, 12);
}
//格式化 恢复到啥都没有的干净状态 先擦数据区 再擦元数据
static void Log_Format(void)
{
    for (uint8_t i = 0; i < LOG_SECTOR_NUM; i++)
    {
        Flash_SectorErase(LOG_DATA_START+i*4096);
    }
    Flash_SectorErase(LOG_META_ADDR);
    s_count=0;
    s_write_addr=LOG_DATA_START;
    Log_SaveMeta();
}
//系统上电  需要通过魔数判断是不是第一次上电
void Log_Init(void)
{
    uint8_t meta[12];
    Flash_ReadData(LOG_META_ADDR,meta,12);
    uint32_t magic;
    magic=(uint32_t)meta[0]<<24;
    magic|=(uint32_t)meta[1]<<16;
    magic|=(uint32_t)meta[2]<<8;
    magic|=(uint32_t)meta[3]<<0;
    if(magic==LOG_MAGIC)//魔数相同
    {
        //恢复记录
        s_count=(uint16_t)meta[4]<<8;
        s_count|=meta[5]<<0;
        s_write_addr = ((uint32_t)meta[8] << 16) | ((uint32_t)meta[9] << 8) | meta[10];
          //防御性检查
        if (s_write_addr < LOG_DATA_START || s_write_addr >= LOG_DATA_END)
        {
            Log_Format();
        }
    }
   else
   {
        Log_Format();
   }
}

void Log_Append(const uint8_t *data, uint8_t len)
{
    uint8_t rec[LOG_REC_SIZE];//十六个字节长度 16 字节要塞下：序号(2) + 长度(1) + 数据(12) + 校验(1)

    uint16_t seq = s_count;    /* 这条的序号 = 当前的条数（从 0 数起） 也需要打包发送*/ 

    uint8_t sum=0;//计算校验和

    //先判断接收数据是否是12位的，大于12位的截断
    if(len>LOG_DATA_MAX) len=LOG_DATA_MAX;
    //边界检查 看是否超出了最大地址  在有效地址内才写
    if(s_write_addr+LOG_REC_SIZE>LOG_DATA_END)//如果要写的地址加上要写的字节数>总地址
    {
        //全擦除再写
        for (uint8_t i = 0; i < LOG_SECTOR_NUM; i++)
        {
           Flash_SectorErase(LOG_DATA_START+4096*i);
        }
         s_count      = 0;
        s_write_addr = LOG_DATA_START;
    }
    // 两位 序列号
    rec[0]=(uint8_t)(seq>>8);
    rec[1]=(uint8_t)seq>>0;
    //一位 长度
    rec[2]=len;
    //十二位数据
     for (uint8_t i = 0; i < LOG_DATA_MAX; i++)//双重保险 i既需要小于固定的12位数据长度，也需要小于当前传过来的数据长度
    {
        rec[3 + i] = (i < len) ? data[i] : 0x00;
    }
    //一位校验位  校验和：前 15 字节累加（uint8_t 自动溢出取模）
    for (uint8_t i = 0; i < 15; i++) sum += rec[i];
    rec[15] = sum;

    //跨页写
    if((s_write_addr&0xFF)+LOG_REC_SIZE>256)//如果当前页内偏移量+上要写的16个字节大于一次页写的范围(256字节)需要跨页写
    {
        uint8_t first = (uint8_t)(256u - (s_write_addr & 0xFFu));
        Flash_PageProgram(s_write_addr,rec,first);
        Flash_PageProgram((s_write_addr & 0xFFFFFF00u) + 256u,
                          rec + first, (uint8_t)(LOG_REC_SIZE - first));//低八位清0,拿到当前页起始地址，再+256调到下一页
    }
    else
    {
        Flash_PageProgram(s_write_addr, rec, LOG_REC_SIZE);
    }
    s_write_addr += LOG_REC_SIZE;
    s_count++;
    Log_SaveMeta();
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
