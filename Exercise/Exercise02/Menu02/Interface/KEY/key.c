#include "key.h"

volatile  Key_Event_t key_evt=KEY_NONE;

//按键的状态 空闲 消抖 短按 长按
typedef enum 
{
    K_IDLE=0,
    K_DEBOUNCE,
    K_DOWN,
    K_LONG
}Key_State_t;


//封装按键 端口号 pin号 有效电平 短按事件 长按事件 按键状态
typedef struct 
{
    GPIO_TypeDef* key_port;
    uint16_t key_pin;
    GPIO_PinState active;//有效电平
    Key_Event_t evt_short;
    Key_Event_t evt_long;
    Key_State_t key_state;
    uint32_t chgtime;
}Key_Obj_t;

static Key_Obj_t keys[]={
    {KEY_UP_GPIO_Port,KEY_UP_Pin,GPIO_PIN_SET,KEY_UP,KEY_UP},//up高电平有效
    {KEY1_GPIO_Port,KEY1_Pin,GPIO_PIN_RESET,KEY_DOWN,KEY_DOWN},
    {KEY0_GPIO_Port,KEY0_Pin,GPIO_PIN_RESET,KEY_SHORT,KEY_LONG},
};

static void Key_Update(Key_Obj_t*k)
{
    uint8_t down=(HAL_GPIO_ReadPin(k->key_port,k->key_pin)==k->active);//是否按下 如果是就是1
    switch (k->key_state)
    {
    case K_IDLE:
        if(down)//如果按下
        {
            //更新时间 进消抖
            k->chgtime=HAL_GetTick();
            k->key_state =K_DEBOUNCE;
        }
    break;
    case K_DEBOUNCE:
        if(HAL_GetTick()-k->chgtime>50)//如果消抖后确认按下
        {
            k->key_state=K_DOWN;
            k->chgtime=HAL_GetTick();
        }
        else if(!down){k->key_state=K_IDLE;}
    break;
    case K_DOWN:
        if(HAL_GetTick()-k->chgtime>1000)//如果大一1s进入长按
        {
            key_evt=k->evt_long;
            k->chgtime=HAL_GetTick();
            k->key_state=K_LONG;
        }
        else if(!down)//如果先松手 一次短按触发 然后回空闲
        {
            key_evt=k->evt_short;
            k->key_state=K_IDLE;
        }
    break;
    case K_LONG:
        if(HAL_GetTick()-k->chgtime>500)//如果>0.5s连续触发
        {
            key_evt=k->evt_long;
            k->chgtime=HAL_GetTick();
        }
        else if(!down)//如果松手了
        {
            k->key_state=K_IDLE;
        }
    break;
    default:
    k->key_state=K_IDLE;
        break;
    }
}
#define KEY_NUM  (sizeof(keys) / sizeof(keys[0]))
void Key_Scan(void)
{
    uint32_t i;

    for(i = 0; i < KEY_NUM; i++)
    {
        Key_Update(&keys[i]);
    }
}
