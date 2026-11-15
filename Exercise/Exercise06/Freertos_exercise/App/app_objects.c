/* 当前文件职责：信号量/互斥量/队列/事件标志组创建和初始化 */
#include "app_object.h"
/* 队列句柄 */
QueueHandle_t qDisplay;
QueueHandle_t qStorage;
QueueHandle_t qCommRx;
/* 信号量/互斥量句柄 */
SemaphoreHandle_t xSemDmaDone;
SemaphoreHandle_t xMtxLcd;
SemaphoreHandle_t xMtxFlash;
SemaphoreHandle_t xMtxPrintf;
/* 事件组句柄 */
EventGroupHandle_t xEvAlive;
/* 全局变量计数定义和初始化 */
volatile uint32_t g_aliveAcquire = 0;
volatile uint32_t g_aliveDisplay = 0;
volatile uint32_t g_aliveComm    = 0;
volatile uint32_t g_aliveStorage = 0;
volatile uint32_t g_workAcquire  = 0;
volatile uint32_t g_workDisplay  = 0;
volatile uint32_t g_workComm     = 0;
volatile uint32_t g_workStorage  = 0;
volatile uint32_t g_dropDisplay  = 0;
volatile uint32_t g_dropStorage  = 0;

void App_ObjectsInit(void)
{
    /* 队列创建 */
    qDisplay = xQueueCreate(QUEUE_LEN_DISPLAY,sizeof(sample_t));
    configASSERT(qDisplay!=NULL);
    qStorage = xQueueCreate(QUEUE_LEN_STORAGE,sizeof(sample_t));
    configASSERT(qStorage!=NULL);
    qCommRx = xQueueCreate(QUEUE_LEN_COMM_RX,sizeof(uint8_t));
    configASSERT(qCommRx!=NULL);
    /* 信号量/互斥量创建 */
    xSemDmaDone = xSemaphoreCreateBinary();
    configASSERT(xSemDmaDone!=NULL);
    xMtxLcd = xSemaphoreCreateMutex();
    configASSERT( xMtxLcd != NULL );
    xMtxFlash = xSemaphoreCreateMutex();
    configASSERT( xMtxFlash != NULL );
    xMtxPrintf = xSemaphoreCreateMutex();
    configASSERT( xMtxPrintf != NULL );
    /* 事件组创建 */
    xEvAlive = xEventGroupCreate();
    configASSERT( xEvAlive != NULL );
}
