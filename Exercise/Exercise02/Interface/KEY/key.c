#include "key.h"

volatile Key_evt_t key_evt=KEY_NONE;

static void Key0_Scan(void);
static void Key1_Scan(void);
static void KeyUp_Scan(void);

void Key_Scan(void)
{
    KeyUp_Scan();
    Key1_Scan();
    Key0_Scan();
}



typedef enum
{
    KEYUP_IDLE=0,
    KEYUP_DEBOUNCE,
    KEYUP_DOWN,
    KEYUP_LONG
}KeyUp_State_t;
static void KeyUp_Scan(void)
{
    static KeyUp_State_t KeyUp_State=KEYUP_IDLE;//KeyUp的状态标志
    static uint32_t KeyUp_chgtime=0;
    uint8_t down=(HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin)==GPIO_PIN_SET);
    //uint8_t KeyUp_now=HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin);
    switch (KeyUp_State)
    {
        case KEYUP_IDLE:
        if(down)//如果变化
        {
            KeyUp_chgtime=HAL_GetTick();//记下当前时间
            KeyUp_State=KEYUP_DEBOUNCE;
        }
        break;
        case KEYUP_DEBOUNCE:
        if(HAL_GetTick()-KeyUp_chgtime>50)
        {
            KeyUp_chgtime=HAL_GetTick();//更新时间
            KeyUp_State=KEYUP_DOWN;
        }
        else if(!down)//如果弹回去了 是抖动
        {
            KeyUp_State=KEYUP_IDLE;
        }
        break;
        case KEYUP_DOWN:
        if(HAL_GetTick()-KeyUp_chgtime>1000)//如果大于1s进入长按
        {
            key_evt=KEY_UP;
            //printf("keyup long\r\n");
            KeyUp_chgtime=HAL_GetTick();//更新一次时间
            KeyUp_State=KEYUP_LONG;
        }
        else if(!down)//如果先1s 松手 短按
        {
            //printf("keyup down\r\n");
            key_evt=KEY_UP;
            KeyUp_State=KEYUP_IDLE;
        }
        break;
        case KEYUP_LONG:
        if(!down)//如果松手
        {
            KeyUp_State=KEYUP_IDLE;
        }
        else if(HAL_GetTick()-KeyUp_chgtime>500)//再来判断一次长按
        {
            key_evt=KEY_UP;
            //printf("keyup long\r\n");
            KeyUp_chgtime=HAL_GetTick();
        }
        break;
    default:
        break;
    }
}

typedef enum
{
    KEY0_IDLE=0,
    KEY0_DEBOUNCE,
    KEY0_DOWN,
    KEY0_LONG
}Key0_State_t;
static void Key0_Scan(void)
{
    static Key0_State_t Key0_State=KEY0_IDLE;//KeyUp的状态标志
    static uint32_t Key0_chgtime=0;
    uint8_t down=(HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin)==GPIO_PIN_RESET);
    //uint8_t KeyUp_now=HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin);
    switch (Key0_State)
    {
        case KEY0_IDLE:
        if(down)//如果变化
        {
            Key0_chgtime=HAL_GetTick();//记下当前时间
            Key0_State=KEY0_DEBOUNCE;
        }
        break;
        case KEY0_DEBOUNCE:
        if(HAL_GetTick()-Key0_chgtime>50)
        {
            Key0_chgtime=HAL_GetTick();//更新时间
            Key0_State=KEY0_DOWN;
        }
        else if(!down)//如果弹回去了 是抖动
        {
            Key0_State=KEY0_IDLE;
        }
        break;
        case KEY0_DOWN:
        if(HAL_GetTick()-Key0_chgtime>1000)//如果大于1s进入长按
        {
            key_evt=KEY_LONG;
            //printf("key0 long\r\n");
            Key0_chgtime=HAL_GetTick();//更新一次时间
            Key0_State=KEY0_LONG;
        }
        else if(!down)//如果先1s 松手 短按
        {
            //printf("key0 down\r\n");
            key_evt=KEY_SHORT;
            Key0_State=KEY0_IDLE;
        }
        break;
        case KEY0_LONG:
        if(!down)//如果松手
        {
            Key0_State=KEY0_IDLE;
        }
        else if(HAL_GetTick()-Key0_chgtime>500)//再来判断一次长按
        {
            key_evt=KEY_LONG;
            //printf("key0 long\r\n");
            Key0_chgtime=HAL_GetTick();
        }
        break;
    default:
        break;
    }
}

typedef enum
{
    KEY1_IDLE=0,
    KEY1_DEBOUNCE,
    KEY1_DOWN,
    KEY1_LONG
}Key1_State_t;
static void Key1_Scan(void)
{
    static Key1_State_t Key1_State=KEY1_IDLE;//KeyUp的状态标志
    static uint32_t Key1_chgtime=0;
    uint8_t down=(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)==GPIO_PIN_RESET);
    //uint8_t KeyUp_now=HAL_GPIO_ReadPin(KEY_UP_GPIO_Port,KEY_UP_Pin);
    switch (Key1_State)
    {
        case KEY1_IDLE:
        if(down)//如果变化
        {
            Key1_chgtime=HAL_GetTick();//记下当前时间
            Key1_State=KEY1_DEBOUNCE;
        }
        break;
        case KEY1_DEBOUNCE:
        if(HAL_GetTick()-Key1_chgtime>50)
        {
            Key1_chgtime=HAL_GetTick();//更新时间
            Key1_State=KEY1_DOWN;
        }
        else if(!down)//如果弹回去了 是抖动
        {
            Key1_State=KEY1_IDLE;
        }
        break;
        case KEY1_DOWN:
        if(HAL_GetTick()-Key1_chgtime>1000)//如果大于1s进入长按
        {
            key_evt=KEY_DOWN;
            //printf("key1 long\r\n");
            Key1_chgtime=HAL_GetTick();//更新一次时间
            Key1_State=KEY1_LONG;
        }
        else if(!down)//如果先1s 松手 短按
        {
            //printf("key1 down\r\n");
            key_evt=KEY_DOWN;
            Key1_State=KEY1_IDLE;
        }
        break;
        case KEY1_LONG:
        if(!down)//如果松手
        {
            Key1_State=KEY1_IDLE;
        }
        else if(HAL_GetTick()-Key1_chgtime>500)//再来判断一次长按
        {
            key_evt=KEY_DOWN;
            //printf("key1 long\r\n");
            Key1_chgtime=HAL_GetTick();
        }
        break;
    default:
        break;
    }
}
