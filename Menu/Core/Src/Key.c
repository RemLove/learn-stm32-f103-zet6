#include "Key.h"
#include "Menu.h"

/* 四个按键：Key_up(KEY_UP=PB11)  Key_down(KEY_DOWN=PB1)
 *          Key_ok(KEY_OK=PA6)     Key_return(KEY_RETURN=PA4)
 * 采用「按下置标志、松开触发」的边沿检测，无硬件消抖时配合主循环周期扫描 */

static int flag_up = 0;
static int flag_down = 0;
static int flag_ok = 0;
static int flag_return = 0;

void Key_Scan(void)
{
    /* UP */
    if (HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET)
    {
        flag_up = 1;
    }
    else
    {
        if (flag_up == 1)
        {
            Key_Up();
            flag_up = 0;
        }
    }

    /* DOWN */
    if (HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET)
    {
        flag_down = 1;
    }
    else
    {
        if (flag_down == 1)
        {
            Key_Down();
            flag_down = 0;
        }
    }

    /* OK */
    if (HAL_GPIO_ReadPin(KEY_OK_GPIO_Port, KEY_OK_Pin) == GPIO_PIN_RESET)
    {
        flag_ok = 1;
    }
    else
    {
        if (flag_ok == 1)
        {
            Key_Enter();
            flag_ok = 0;
        }
    }

    /* RETURN */
    if (HAL_GPIO_ReadPin(KEY_RETURN_GPIO_Port, KEY_RETURN_Pin) == GPIO_PIN_RESET)
    {
        flag_return = 1;
    }
    else
    {
        if (flag_return == 1)
        {
            Key_Back();
            flag_return = 0;
        }
    }
}
