#include "menu.h"
#include "gpio.h"
#include "tim.h"

/*==============================================================
 *  1. 屏幕布局
 *============================================================*/
#define VISIBLE_ROWS   4        /* 一屏最多显示几行 */
#define TEXT_X         20       /* 条目文字的 x 坐标 */
#define CURSOR_X       0        /* "->" 光标的 x 坐标 */

/* 行坐标表：4 行的 y 值永远是这四个。
   滚动靠的是"条目在里面滑"，不是改坐标。 */
static const uint16_t row_y[VISIBLE_ROWS] = {100, 130, 160, 190};

/* 求数组元素个数，省得手写数字 */
#define ITEM_CNT(a)    ((uint8_t)(sizeof(a) / sizeof((a)[0])))


/*==============================================================
 *  2. 数据结构
 *============================================================*/
typedef struct Menu_Page Menu_Page_t;

/* 一个选项：名字 + 下一级页面(可选) + 动作(可选) */
typedef struct
{
    const char       *name;
    const Menu_Page_t *child;       /* 不为 NULL = 这是个文件夹 */
    void            (*action)(void);/* 叶子节点按下后干什么 */
} Menu_Item_t;

/* 一页：标题 + 选项数组 + 选项数 + 三个钩子 + 刷新周期 */
struct Menu_Page
{
    const char        *title;
    const Menu_Item_t *item;
    uint8_t            count;       /* 0 = 纯显示页（TIME / INFO） */

    void    (*on_enter)(void);      /* 进入本页时调一次 */
    void    (*draw_extra)(void);    /* 额外内容，也会被 refresh_ms 定时调用 */
    void    (*key_hook)(void);      /* 不为 NULL = 本页自己接管按键（PWM 用） */
    uint16_t  refresh_ms;           /* 定时刷新周期，0 = 不刷新 */
};


/*==============================================================
 *  3. 页面定义
 *     被引用的要先定义，所以从最深那层往外写
 *============================================================*/

/*------------------- LED 的动作 -------------------*/
/* LED1 = PE5，普通 GPIO，低电平点亮 */
static void Led1_On(void)  { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); }
static void Led1_Off(void) { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   }

/* LED2 = PB5，接在 TIM3_CH2 上，靠占空比控制亮度 */
static void Led2_On(void)  { __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 100); }  /* 100% 最亮 */
static void Led2_Off(void) { __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);   }  /* 0% 灭 */

/*------------------- 第三层：LED1 -------------------*/
static const Menu_Item_t led1_item[] =
{
    {"on",  NULL, Led1_On },
    {"off", NULL, Led1_Off},
};
static const Menu_Page_t led1_page =
{
    "LED1", led1_item, ITEM_CNT(led1_item), NULL, NULL, NULL, 0
};

/*------------------- 第三层：LED2 -------------------*/
static const Menu_Item_t led2_item[] =
{
    {"on",  NULL, Led2_On },
    {"off", NULL, Led2_Off},
};
static const Menu_Page_t led2_page =
{
    "LED2", led2_item, ITEM_CNT(led2_item), NULL, NULL, NULL, 0
};

/*------------------- 第二层：LED -------------------*/
static const Menu_Item_t led_item[] =
{
    {"LED1", &led1_page, NULL},
    {"LED2", &led2_page, NULL},
};
static const Menu_Page_t led_page =
{
    "LED", led_item, ITEM_CNT(led_item), NULL, NULL, NULL, 0
};


/*------------------- PWM 页（编辑模式） -------------------*/
#define PWM_MAX   100       /* 占空比上限 100% */

/* 进度条的屏幕位置 */
#define BAR_X     20
#define BAR_Y     165
#define BAR_W     200
#define BAR_H     20

static uint8_t  pwm_saved = 0;      /* 已经写进硬件的值 */
static uint8_t  pwm_edit  = 0;      /* 正在编辑的临时值 */
static uint32_t pwm_last_tick = 0;  /* 上一次按上/下键的时刻，用来判连击 */
static uint8_t  pwm_acc = 0;        /* 连击计数，用来加速 */

static void pwm_enter(void);
static void pwm_draw(void);
static void pwm_key(void);

static const Menu_Page_t pwm_page =
{
    "pwm", NULL, 0, pwm_enter, pwm_draw, pwm_key, 0
};


/*------------------- TIME 页（纯显示） -------------------*/
static void time_draw(void)
{
    char     buf[24];
    uint32_t sec = HAL_GetTick() / 1000;

    uint8_t hh = (uint8_t)((sec / 3600) % 100);   /* 时，超过 99 小时回绕 */
    uint8_t mm = (uint8_t)((sec % 3600) / 60);    /* 分 */
    uint8_t ss = (uint8_t)(sec % 60);             /* 秒 */

    sprintf(buf, "%02d:%02d:%02d", hh, mm, ss);
    LCD_WriteAsiciStr(20, 120, 24, WHITE, BLACK, (uint8_t *)buf);

    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"long OK = back");
}
static const Menu_Page_t time_page =
{
    "time", NULL, 0, NULL, time_draw, NULL, 500
};


/*------------------- INFO 页（纯显示） -------------------*/
static void info_draw(void)
{
    static uint32_t lcd_id = 0;     /* 只在第一次读，之后用缓存 */
    char buf[24];

    if(lcd_id == 0)
    {
        lcd_id = LCD_ReadId();
    }

    sprintf(buf, "LCD ID = %#x", (unsigned)lcd_id);
    LCD_WriteAsiciStr(0, 110, 16, WHITE, BLACK, (uint8_t *)buf);

    sprintf(buf, "Tick = %lu", (unsigned long)HAL_GetTick());
    LCD_WriteAsiciStr(0, 140, 16, WHITE, BLACK, (uint8_t *)buf);

    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"long OK = back");
}
static const Menu_Page_t info_page =
{
    "info", NULL, 0, NULL, info_draw, NULL, 500
};


/*------------------- HELP 页（纯显示，顺便让主菜单超过一屏） -------------------*/
static void help_draw(void)
{
    LCD_WriteAsiciStr(0,  95, 16, WHITE, BLACK, (uint8_t *)"UP/DOWN  move");
    LCD_WriteAsiciStr(0, 125, 16, WHITE, BLACK, (uint8_t *)"SHORT OK enter");
    LCD_WriteAsiciStr(0, 155, 16, WHITE, BLACK, (uint8_t *)"LONG  OK back");

    LCD_WriteAsiciStr(0, 195, 16, GRAY, BLACK, (uint8_t *)"list scrolls when");
    LCD_WriteAsiciStr(0, 215, 16, GRAY, BLACK, (uint8_t *)"items > 4");

    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"long OK = back");
}
static const Menu_Page_t help_page =
{
    "help", NULL, 0, NULL, help_draw, NULL, 0
};


/*------------------- 第一层：主菜单 -------------------*/
static const Menu_Item_t main_item[] =
{
    {"LED",  &led_page,  NULL},
    {"PWM",  &pwm_page,  NULL},
    {"TIME", &time_page, NULL},
    {"INFO", &info_page, NULL},
    {"HELP", &help_page, NULL},
};
static const Menu_Page_t main_page =
{
    "Menu", main_item, ITEM_CNT(main_item), NULL, NULL, NULL, 0
};


/*==============================================================
 *  4. 运行时状态：返回栈 + 光标 + 滚动窗口
 *============================================================*/
#define MAX_DEPTH   4

static const Menu_Page_t *page_stack[MAX_DEPTH];   /* 每一层是哪个页面 */
static uint8_t            sel_stack[MAX_DEPTH];    /* 每一层选中第几项 */
static uint8_t            off_stack[MAX_DEPTH];    /* 每一层的滚动偏移 */
static uint8_t            depth = 0;

static const Menu_Page_t *cur_page   = &main_page;
static uint8_t            cur_sel    = 0;   /* 选中第几项（绝对下标，不是行号） */
static uint8_t            scroll_off = 0;   /* 窗口第一行显示第几项 */


/*==============================================================
 *  5. 画
 *============================================================*/
static void Menu_DrawPage(void);    /* 下面 Cursor_Move 要用，先声明 */

static void Cursor_Set(uint16_t y, uint16_t color)
{
    LCD_WriteAsiciStr(CURSOR_X, y, 16, color, BLACK, (uint8_t *)"->");
}

/* 让窗口刚好罩住 cur_sel：选中项跑出窗口就把窗口推过去 */
static void Window_Fit(void)
{
    if(cur_sel < scroll_off)
    {
        scroll_off = cur_sel;                                   /* 跑到窗口上面了 */
    }
    else if(cur_sel >= scroll_off + VISIBLE_ROWS)
    {
        scroll_off = (uint8_t)(cur_sel - VISIBLE_ROWS + 1);      /* 跑到窗口下面了 */
    }
}

/* 移动光标 */
static void Cursor_Move(Key_Event_t evt)
{
    uint8_t old_off = scroll_off;

    /* 1. 用"旧窗口"的行号擦掉旧光标 */
    Cursor_Set(row_y[cur_sel - scroll_off], BLACK);

    /* 2. 算新的选中项，到头就绕回去 */
    if(evt == KEY_DOWN)
    {
        cur_sel = (cur_sel + 1 >= cur_page->count) ? 0 : (uint8_t)(cur_sel + 1);
    }
    else
    {
        cur_sel = (cur_sel == 0) ? (uint8_t)(cur_page->count - 1) : (uint8_t)(cur_sel - 1);
    }

    /* 3. 窗口跟着走 */
    Window_Fit();

    /* 4. 窗口动过 → 所有条目都移位了，只能整页重画；
          没动 → 补一个新光标就够了 */
    if(scroll_off != old_off)
    {
        Menu_DrawPage();
    }
    else
    {
        Cursor_Set(row_y[cur_sel - scroll_off], RED);
    }
}

/* 画出当前整个页面 */
static void Menu_DrawPage(void)
{
    char    buf[24];
    uint8_t r;

    LCD_ClearAll(BLACK);

    /* 标题 */
    LCD_WriteAsiciStr(0, 30, 24, YELLOW, BLACK, (uint8_t *)cur_page->title);

    /* 标题下的分隔线 */
    LCD_DrawLine(30, 80, 210, 80, 1, GRAY);

    /* 只画窗口里的 VISIBLE_ROWS 行 */
    for(r = 0; r < VISIBLE_ROWS; r++)
    {
        uint8_t idx = (uint8_t)(scroll_off + r);

        if(idx >= cur_page->count) break;       /* 条目不够 4 个，剩下的不画 */

        sprintf(buf, "%d.%s", idx + 1, cur_page->item[idx].name);
        LCD_WriteAsiciStr(TEXT_X, row_y[r], 16, WHITE, BLACK, (uint8_t *)buf);
    }

    /* 底部分隔线 */
    LCD_DrawLine(30, 226, 210, 226, 1, GRAY);

    /* 光标画在选中项所在的那一行 */
    if(cur_page->count > 0)
    {
        Cursor_Set(row_y[cur_sel - scroll_off], RED);
    }

    /* 页面自己的额外内容 */
    if(cur_page->draw_extra != NULL)
    {
        cur_page->draw_extra();
    }
}


/*==============================================================
 *  6. 进出页面
 *============================================================*/
static void Menu_Go(const Menu_Page_t *p)
{
    if(p == NULL)          return;
    if(depth >= MAX_DEPTH) return;              /* 栈满了，不进去 */

    /* 记下当前页面的现场 */
    page_stack[depth] = cur_page;
    sel_stack[depth]  = cur_sel;
    off_stack[depth]  = scroll_off;
    depth++;

    /* 切到新页面 */
    cur_page   = p;
    cur_sel    = 0;
    scroll_off = 0;

    if(cur_page->on_enter != NULL)
    {
        cur_page->on_enter();
    }

    Menu_DrawPage();
}

static void Menu_Back(void)
{
    if(depth == 0) return;                      /* 已经在顶层，没得退 */

    depth--;
    cur_page   = page_stack[depth];             /* 恢复页面 */
    cur_sel    = sel_stack[depth];              /* 恢复选中的项 */
    scroll_off = off_stack[depth];              /* 恢复滚动位置 */

    Menu_DrawPage();
}


/*==============================================================
 *  7. 对外接口
 *============================================================*/
void Menu_Init(void)
{
    LCD_BGOn();

    depth      = 0;
    cur_page   = &main_page;
    cur_sel    = 0;
    scroll_off = 0;

    Menu_DrawPage();
}

void Menu_Scan(void)
{
    static uint32_t last_refresh = 0;

    /* ---- 1. 需要定时刷新的页面（TIME / INFO） ---- */
    if(cur_page->refresh_ms != 0 && cur_page->draw_extra != NULL)
    {
        if((uint32_t)(HAL_GetTick() - last_refresh) >= cur_page->refresh_ms)
        {
            last_refresh = HAL_GetTick();
            cur_page->draw_extra();
        }
    }

    /* ---- 2. 有页面要自己接管按键（PWM 编辑页） ---- */
    if(cur_page->key_hook != NULL)
    {
        cur_page->key_hook();
        key_evt = KEY_NONE;
        return;
    }

    /* ---- 3. 纯显示页（count == 0）：只认长按返回 ---- */
    if(cur_page->count == 0)
    {
        if(key_evt == KEY_LONG)
        {
            Menu_Back();
        }
        key_evt = KEY_NONE;
        return;
    }

    /* ---- 4. 上下移动光标 ---- */
    if(key_evt == KEY_UP || key_evt == KEY_DOWN)
    {
        Cursor_Move(key_evt);
        key_evt = KEY_NONE;
        return;
    }

    /* ---- 5. 确定 / 返回 ---- */
    switch(key_evt)
    {
    case KEY_SHORT:
    {
        key_evt = KEY_NONE;
        {
            const Menu_Item_t *it = &cur_page->item[cur_sel];

            if(it->child == NULL && it->action != NULL)
            {
                it->action();               /* 叶子：执行动作 */
            }
            else
            {
                Menu_Go(it->child);         /* 文件夹：进下一层 */
            }
        }
        break;
    }

    case KEY_LONG:
        key_evt = KEY_NONE;
        Menu_Back();
        break;

    default:
        key_evt = KEY_NONE;
        break;
    }
}


/*==============================================================
 *  8. PWM 页的实现
 *============================================================*/

/* 进入页面：从硬件读回真实值，保证和 LED 页改过的一致 */
static void pwm_enter(void)
{
    pwm_saved     = (uint8_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2);
    pwm_edit      = pwm_saved;
    pwm_acc       = 0;
    pwm_last_tick = 0;
}

/* 只画会变的东西：数值 + 进度条 + 底部提示 */
static void pwm_draw(void)
{
    char     buf[24];
    uint32_t fill;

    sprintf(buf, "duty = %3d%%", pwm_edit);     /* %3d 让位数变化时不左右跳 */
    LCD_WriteAsiciStr(20, 120, 16, WHITE, BLACK, (uint8_t *)buf);

    /* 1. 边框 */
    LCD_DrawCFX(BAR_X, BAR_Y, BAR_X + BAR_W, BAR_Y + BAR_H, 1, WHITE);
    /* 2. 内部抹黑 */
    LCD_DrawFill(BAR_X + 1, BAR_Y + 1, BAR_W - 1, BAR_H - 1, BLACK);
    /* 3. 按百分比填绿 */
    fill = (uint32_t)pwm_edit * (BAR_W - 1) / 100;
    if(fill > 0)
    {
        LCD_DrawFill(BAR_X + 1, BAR_Y + 1, (uint16_t)fill, BAR_H - 1, GREEN);
    }

    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"OK=save  long=cancel");
}

/* PWM 接管按键：上下改临时值、短按保存、长按取消 */
static void pwm_key(void)
{
    uint8_t  step;
    uint32_t now = HAL_GetTick();

    switch(key_evt)
    {
    case KEY_UP:
    case KEY_DOWN:
        /* 长按时按键模块每 500ms 补发一次事件。
           两次间隔 < 650ms 说明手一直按着 —— 累加计数，
           每满 6 次步长 +1，最快一次跳 10 格。 */
        if(now - pwm_last_tick < 650)
        {
            if(pwm_acc < 60) pwm_acc++;
        }
        else
        {
            pwm_acc = 0;
        }
        pwm_last_tick = now;

        step = (uint8_t)(1 + pwm_acc / 6);
        if(step > 10) step = 10;

        if(key_evt == KEY_UP)
        {
            pwm_edit = (pwm_edit + step > PWM_MAX) ? PWM_MAX
                                                   : (uint8_t)(pwm_edit + step);
        }
        else
        {
            pwm_edit = (pwm_edit < step) ? 0 : (uint8_t)(pwm_edit - step);
        }
        pwm_draw();
        break;

    case KEY_SHORT:                     /* 保存：写进硬件，返回 */
        pwm_saved = pwm_edit;
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_saved);
        Menu_Back();
        break;

    case KEY_LONG:                      /* 取消：丢弃临时值，硬件不动 */
        pwm_edit = pwm_saved;
        Menu_Back();
        break;

    default:
        break;
    }
}
