/* 当前文件职责：攒页写 256/16=16 16条/页 写前擦 统计seq */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "app_object.h"
#include "app_task.h"
#include "flash.h"
#include <string.h>
/* 宏定义 */
#define FLASH_TOTAL_SIZE (0x1000000u)                   /* 16MB 总容量 */
#define SECTOR_SIZE (4096u)                             /* 4KB 扇区：擦除最小单位，约 45ms */
#define PAGE_SIZE (256u)                                /* 页编程上限 256B，编程约 3ms */


static uint8_t s_page_buff[PAGE_SIZE];
static uint16_t s_fill = 0;     // 记录缓冲区到哪了
static uint32_t s_wr_addr = 0; // 地址

static void TaskStorage(void *pv)
{
    (void)pv;
    Flash_Init();

    for (;;)
    {
        /* ★ 带超时接收，不能 portMAX_DELAY
         * 队列断流时（采集任务停了）portMAX_DELAY 会把这个任务永久挂起，
         * 循环末尾的心跳永远不执行 → monitor 判"冻死" → 不喂狗 → 2s 复位。
         * 带超时后：最多 100ms 返回一次，攒不够一页就先转圈把心跳打上。*/
        sample_t s;
        if (xQueueReceive(qStorage, &s, pdMS_TO_TICKS(APP_WAIT_MS)) == pdPASS)
        {
            memcpy(&s_page_buff[s_fill], &s, sizeof(sample_t));
            s_fill += sizeof(sample_t);
            if (s_fill >= PAGE_SIZE)
            {
                if (s_wr_addr % SECTOR_SIZE == 0)
                {
                    xSemaphoreTake(xMtxFlash, portMAX_DELAY);
                    Flash_SectorErase(s_wr_addr);   /* 最长约 400ms，纯阻塞轮询 */
                    xSemaphoreGive(xMtxFlash);
                }
                xSemaphoreTake(xMtxFlash, portMAX_DELAY);
                Flash_PageProgram(s_wr_addr, s_page_buff, PAGE_SIZE);
                xSemaphoreGive(xMtxFlash);

                s_wr_addr += PAGE_SIZE;
                if(s_wr_addr>=FLASH_TOTAL_SIZE) {s_wr_addr=0;}
                s_fill = 0;
            }
            /* 业务量：真收到一条记录才算 */
            g_workStorage++;
        }
        /* 心跳：无条件 +1，证明本任务还在转 */
        g_aliveStorage++;
    }
}

TaskHandle_t xTaskStorage = NULL;

void TaskStorage_Create(void)
{
    BaseType_t ok = xTaskCreate(TaskStorage, "Storage",
                                TASK_STACK_STORAGE, NULL,
                                TASK_PRIO_STORAGE, &xTaskStorage);
    configASSERT(ok == pdPASS);
}
