#include "key.h"
#include "gpio.h"
 volatile uint8_t keyUp_flag=0;//判断keyUp是否按下的标志
 volatile uint8_t key0_flag=0;//判断key0是否按下的标志
 volatile uint8_t key1_flag=0;//判断key1是否按下的标志

static uint8_t key0_last=GPIO_PIN_SET;//默认是高电平 
uint32_t key0_lasttime=0;

static uint8_t key1_last=GPIO_PIN_SET;//默认是高电平 
uint32_t key1_lasttime=0;

static uint8_t keyup_last=GPIO_PIN_RESET;//默认是低电平 
uint32_t keyup_lasttime=0;


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
      //1.状态机消抖
    uint8_t key0_now=HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);//获取当前电平
    if(key0_last!=key0_now)//如果电平发生变化
    {  
        uint32_t key0_time=HAL_GetTick();//记录当前时间
        if(key0_time-key0_lasttime>50)//如果当前时间-上次时间>50
        {
            //更新时间
            key0_lasttime=key0_time;
            if(key0_now!=key0_last)//如果电平真的改变了
            {
                if(key0_now==GPIO_PIN_RESET)
                {
                    key0_flag=1;//按下
                }
                else 
                {
                    key0_flag=0;
                }
                key0_last=key0_now;
            }
        }
    }
}
static void key1_scan(void)
{
      //1.状态机消抖
    uint8_t key1_now=HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin);//获取当前电平
    if(key1_last!=key1_now)//如果电平发生变化
    {  
        uint32_t key1_time=HAL_GetTick();//记录当前时间
        if(key1_time-key1_lasttime>50)//如果当前时间-上次时间>50
        {
            //更新时间
            key1_lasttime=key1_time;
            if(key1_now!=key1_last)//如果电平真的改变了
            {
                if(key1_now==GPIO_PIN_RESET)
                {
                    key1_flag=1;//按下
                }
                else 
                {
                    key1_flag=0;
                }
                key1_last=key1_now;
            }
        }
    }
}
static void keyUp_scan(void)
{
      //1.状态机消抖
    uint8_t keyup_now=HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin);//获取当前电平
    if(keyup_last!=keyup_now)//如果电平发生变化
    {  
        uint32_t keyup_time=HAL_GetTick();//记录当前时间
        if(keyup_time-keyup_lasttime>50)//如果当前时间-上次时间>50
        {
            //更新时间
            keyup_lasttime=keyup_time;
            if(keyup_now!=keyup_last)//如果电平真的改变了
            {
                if(keyup_now==GPIO_PIN_SET)
                {
                    keyUp_flag=1;//按下
                }
                else 
                {
                    keyUp_flag=0;
                }
                keyup_last=keyup_now;
            }
        }
    }
}
