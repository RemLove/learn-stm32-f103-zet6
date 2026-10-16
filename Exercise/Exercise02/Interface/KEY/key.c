#include "key.h"

volatile uint8_t keyUp_flag=0;//判断keyUp是否按下的标志
volatile uint8_t key0_flag=0;//判断key0是否按下的标志
volatile uint8_t key1_flag=0;//判断key1是否按下的标志

static uint8_t key0_stable = GPIO_PIN_SET;  // 记录消抖后的稳定电平
static uint8_t key0_last=GPIO_PIN_SET;//默认是高电平 
uint32_t key0_lasttime=0;

static uint8_t key1_stable = GPIO_PIN_SET;  // 记录消抖后的稳定电平
static uint8_t key1_last=GPIO_PIN_SET;//默认是高电平 
uint32_t key1_lasttime=0;

static uint8_t keyUp_stable = GPIO_PIN_RESET;  // 记录消抖后的稳定电平
static uint8_t keyUp_last=GPIO_PIN_RESET;//默认是低电平 
uint32_t keyUp_lasttime=0;


static void key0_scan(void);
static void key1_scan(void);
static void keyUp_scan(void);


void Key_Scan(void)
{
  key0_scan();
  key1_scan();
  keyUp_scan();
}

static void key0_scan(void)
{
    //状态机消抖
    //1.看电平是否改变
    uint8_t key0_now=HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);
    if(key0_last!=key0_now)//如果电平改变
    {
        //记录时间和更新电平
        key0_lasttime=HAL_GetTick();
        key0_last=key0_now;
    }
    if(HAL_GetTick()-key0_lasttime>50)//如果50ms了没有改变
    {
        //判断现在电平和稳定电平
        if(key0_now!=key0_stable)//如果不相等 发生改变了,放在长按触发
        {
            key0_stable=key0_now;
            if(key0_stable==GPIO_PIN_RESET)
            {
                key0_flag=1;
            }
        }
    }
}

static void key1_scan(void)
{
    //状态机消抖
    //1.判断是否电平发生改变
    uint8_t key1_now=HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);
    if(key1_last!=key1_now)//电平发生改变
    {
        //更新电平记录时间
        key1_lasttime=HAL_GetTick();
        key1_last=key1_now;
    }
    //2.判断稳定时间是否>50ms
    if(HAL_GetTick()-key1_lasttime>50)
    {
        //3.如果稳定 看看现在电平和稳定电平是否相同
        if(key1_now!=key1_stable)//防止长按的
        {
            key1_stable=key1_now;
            if(key1_stable==GPIO_PIN_RESET)
            {
                key1_flag=1;
            }
        }
        //如果现在电平和稳定电平相同  nothing
    }
}
static void keyUp_scan(void)
{
    //状态机消抖
    //1.判断是否电平发生改变
    uint8_t keyUp_now=HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin);
    if(keyUp_last!=keyUp_now)//电平发生改变
    {
        //更新电平记录时间
        keyUp_lasttime=HAL_GetTick();
        keyUp_last=keyUp_now;
    }
    //2.判断稳定时间是否>50ms
    if(HAL_GetTick()-keyUp_lasttime>50)
    {
        //3.如果稳定 看看现在电平和稳定电平是否相同
        if(keyUp_now!=keyUp_stable)//防止长按的
        {
            keyUp_stable=keyUp_now;
            if(keyUp_stable==GPIO_PIN_SET)
            {
                keyUp_flag=1;
            }
        }
        //如果现在电平和稳定电平相同  nothing
    }
}
