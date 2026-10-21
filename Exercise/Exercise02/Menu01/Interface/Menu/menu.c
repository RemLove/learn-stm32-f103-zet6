#include "menu.h"

#define MENU_COUNT 4
//各层的坐标
static const uint16_t menu_y[]= {100,130,160,190};//主菜单四行
static const uint16_t two_y[] = {120, 160};//两行共用的


/* menu_item_t 里要指向 menu_page_t，两个结构互相引用 */
typedef struct menu_page menu_page_t;

typedef struct 
{
    const char  *text;
    const menu_page_t *child;//不为NULL 则是文件夹
    void (*action)(void);//实现
}menu_item_t;

struct menu_page
{
    const char*         title;
    const menu_item_t *items;   //相当于文件夹
    uint8_t            count;   /* 有几行 */
    const uint16_t    *ys;      /* 每行在屏幕上的 y 坐标 */

    //------可选钩子---------
    void(*on_enter)(void);  //每次进入pwm需要现在pwm的原先值
    void(*draw_extra)(void);//需要额外画出pwm值
    void(*pwm_key)(void);   //pwm是编辑模式，上下键将变成+1 和-1键 短确定是确定pwm值，长按是取消pwm值
};

//LED 灯的亮灭
static void Led1_On(void)  { __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 100); }  /* 100% 最亮 */
static void Led1_Off(void) { __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);   }  /* 0% 灭 */

static void Led2_On(void)   { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); }
static void Led2_Off(void)  { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   }

//-------------led--------------
//定义顺序：被引用的必须先定义 所以需要从最深往外定义
//第三层
//1.LED1
static const menu_item_t LED1_item[]={
    {"on",NULL,Led1_On},
    {"off",NULL,Led1_Off}
};
static const menu_page_t led1_page={"LED1",LED1_item,2,two_y,NULL,NULL,NULL};//led1_page

//2.LED2
static const menu_item_t LED2_item[]={
    {"on",NULL,Led2_On},
    {"off",NULL,Led2_Off}
};
static const menu_page_t led2_page={"LED2",LED2_item,2,two_y,NULL,NULL,NULL};//led2_page
//第一层
static const menu_item_t LED_item[]={
    {"LED1",&led1_page,NULL},
    {"LED2",&led2_page,NULL}
};
static const menu_page_t LED_page={"LED",LED_item,2,two_y,NULL,NULL,NULL};//led_page
//-------pwm---------
static void pwm_enter(void);
static void pwm_draw(void);
static void pwm_key(void);
/* PWM 进度条的屏幕位置 */
#define BAR_X   20      /* 左上角 x */
#define BAR_Y   165     /* 左上角 y */
#define BAR_W   200     /* 总宽 */
#define BAR_H   20      /* 总高 */
static const menu_page_t pwm_page={"pwm",NULL,0,NULL,pwm_enter,pwm_draw,pwm_key};
//-------info---------
static void info_draw(void);
static const menu_page_t info_page={"info",NULL,0,NULL,NULL,info_draw,NULL};
//-------TIME---------
static void time_draw(void);
static const menu_page_t time_page={"time",NULL,0,NULL,NULL,time_draw,NULL};


static  menu_item_t main_items[] = {
    { "1.LED",   &LED_page,   NULL },
    { "2.PWM",   &pwm_page,   NULL },   /* 还没做，确定后无反应 */
    { "3.TIME",  &time_page,  NULL },
    { "4.INFO",  &info_page,  NULL },
};
static const menu_page_t main_page = { "Menu", main_items, 4, menu_y,NULL,NULL,NULL };//main_page


//运行时栈来记录
#define STACK_DEPTH 4
static const menu_page_t* page_stack[STACK_DEPTH];//返回路径：记下每层的页面
static uint8_t      sel_stack[STACK_DEPTH];//返回路径：记下每层的页面的第几项
static uint8_t      depth=0;              //当前栈深度

static const menu_page_t* cur_page=&main_page;//当前所指向的的页面
static uint8_t      cur_sel=0;//当前选中的是第几行


static void Cursor_Set(uint16_t y, uint16_t color)
{
    LCD_WriteAsiciStr(0, y, 16, color, BLACK, (uint8_t *)"->");
}

static void Cursor_Move(const uint16_t* y_table,uint8_t* index,Key_evt_t key_evt,uint8_t limit ) //需要知道上次光标位置 下次光标位置
{
    //1.清除光标
    Cursor_Set(y_table[*index],BLACK);
    //2.计算下次光标
    if(key_evt==KEY_DOWN)//如果是向下
    {
        *index =(*index==limit-1)? 0:(*index+1);
    }
    else if(key_evt==KEY_UP)//向上
    {
        *index =(*index ==0)?(limit-1):(*index-1);
    }
    //3.画出新的光标
    Cursor_Set(y_table[*index],RED);
}

//画出当前页面
static void Menu_DrawPage(void)
{
     LCD_ClearAll(BLACK);

    /* 标题 */
    LCD_WriteAsiciStr(0, 30, 24, YELLOW, BLACK, (uint8_t *)cur_page->title);

    /* 标题下的分隔线 */
    LCD_DrawLine(30, 80, 210, 80, 1, GRAY);

    /* 逐行写菜单项 */
    for(uint8_t i = 0; i < cur_page->count; i++)
    {
        LCD_WriteAsiciStr(20, cur_page->ys[i], 16, WHITE, BLACK,
                          (uint8_t *)cur_page->items[i].text);
    }

    /* 底部分隔线 */
    LCD_DrawLine(30, 226, 210, 226, 1, GRAY);

    /* 光标画在选中的那一行 */
    if(cur_page->count > 0)
    {
        Cursor_Set(cur_page->ys[cur_sel], RED);
    }
    if(cur_page->draw_extra != NULL)
    {
        cur_page->draw_extra();
    }
}

//进入子文件夹 先压入栈 再切换
static void Menu_Go(const menu_page_t *p)
{
    if(p==NULL) return;
    //需要判断栈是否满
    if(depth<STACK_DEPTH)
    {
        page_stack[depth]=cur_page;
        sel_stack[depth]=cur_sel;
        depth++;
    }
    else {return;}
    cur_page=p;
    cur_sel=0;

    //如果是要进入pwm则需要现在pwm的值
    if(cur_page->on_enter!=NULL) cur_page->on_enter();

    Menu_DrawPage();
}

/* 返回上一页：从栈里弹出来 */
static void Menu_Back(void)
{
    if(depth == 0) return;                 /* 已经在顶层，没得退 */

    depth--;
    cur_page = page_stack[depth];          /* 恢复页面 */
    cur_sel  = sel_stack[depth];           /* 恢复当时选的行 */

    Menu_DrawPage();
}
//对外接口
void Menu_Init(void)
{
    LCD_BGOn();

    depth    = 0;
    cur_page = &main_page;
    cur_sel  = 0;

    Menu_DrawPage();
}

void Menu_Scan(void)
{
    //用于实时刷新time
    static uint32_t last_refresh = 0;

    if(HAL_GetTick() - last_refresh >= 1000)
    {
        last_refresh = HAL_GetTick();

        if(cur_page->draw_extra != NULL)
        {
            cur_page->draw_extra();
        }
    }
    
    //需要判断是否是pwm模式
    if(cur_page->pwm_key!=NULL)
    {
        cur_page->pwm_key();
        key_evt=KEY_NONE;
        return;
    }
    //判断是否是info或者time 是的话 丢弃上下键
    if(cur_page->count == 0)
    {
        if(key_evt == KEY_LONG)
        {
            Menu_Back();
        }

        key_evt = KEY_NONE;      /* 上下键、确定键一律丢弃 */
        return;
    }

    if(key_evt==KEY_UP||key_evt==KEY_DOWN)
    {
        Cursor_Move(cur_page->ys,&cur_sel,key_evt,cur_page->count);
        key_evt=KEY_NONE;
        return;
    }
    switch (key_evt)
    {
        
    case KEY_SHORT:
    {
        key_evt=KEY_NONE;

        const menu_item_t* it=&cur_page->items[cur_sel];

        if(it->child==NULL&&it->action!=NULL)//不是文件夹
        {
            it->action();
        }
        else
        {
            Menu_Go(it->child);
        }
    }
        break;
        
    case KEY_LONG:
        key_evt=KEY_NONE;
        Menu_Back(); 
        break;
    default:
        key_evt=KEY_NONE;
        break;
    }
}
//----------------------------------------P W M 页 面 编 辑-----------------------------------------------------------------------------------
#define PWM_MAX 100 //pwm最大值

static uint8_t pwm_saved=0;//默认是0，这个是已经保存的值
static uint8_t pwm_edit=0;//这个是要编辑的值
//进入pwm页面时需要把临时值拉回保存值
static void pwm_enter(void)
{
    pwm_saved = (uint8_t)__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2);
    pwm_edit=pwm_saved;
}
//pwm额外画的 只画改变的数值就行
static void pwm_draw(void)
{
    char buf[24];          /* 缓冲区，够放 "duty = 100%" 就行 */

    /* sprintf 把数字转成字符串。%3d 表示至少占 3 个字符宽度，
       这样 5 会显示成 "  5"，100 显示成 "100"，
       数字变化时不会因为位数变化导致后面内容左右跳 */
    sprintf(buf, "duty = %3d%%", pwm_edit);   /* %% 表示输出一个 % 字符 */

    LCD_WriteAsiciStr(20, 120, 16, WHITE, BLACK, (uint8_t *)buf);

    //画进度条
    //1.先画边框
    LCD_DrawCFX(BAR_X,BAR_Y,BAR_X+BAR_W,BAR_Y+BAR_H,1,WHITE);
    //2，清除内部
    LCD_DrawFill(BAR_X + 1, BAR_Y + 1, BAR_W - 1, BAR_H - 1, BLACK);
    //3.填充
    uint32_t fill=(uint32_t)(pwm_edit*(BAR_W-1)/100);
     if(fill > 0)
    {
        LCD_DrawFill(BAR_X + 1, BAR_Y + 1, (uint16_t)fill, BAR_H - 1, GREEN);
    }
    
    /* 提示行：告诉用户这两个键在当前页面是干什么的 */
    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"OK=save  long=cancel");
    
}
//pwm接管按键
static void pwm_key(void)
{
    switch (key_evt)
    {
    case KEY_UP ://计数+1
        if(pwm_edit<PWM_MAX) pwm_edit++;
        pwm_draw();
        break;
    case KEY_DOWN://计数-1
        if(pwm_edit>0) pwm_edit--;
        pwm_draw();
        break;
    case KEY_SHORT:
        pwm_saved=pwm_edit;
        __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2,pwm_saved);
        Menu_Back();
        break;
    case KEY_LONG:                    /* 取消：丢弃临时值，硬件不动 */
        pwm_edit = pwm_saved;
        Menu_Back();
        break;
    default:
        break;
    }
}
//----------------------------------------i n f o 页 面 编 辑-----------------------------------------------------------------------------------

static void info_draw(void)
{
    static uint32_t lcd_id = 0; 
    char buf[24];
     if(lcd_id == 0)                  /* 只在第一次读 */
    {
        lcd_id = LCD_ReadId();
    }

    sprintf(buf, "LCD ID = %#x", (unsigned)lcd_id);
    LCD_WriteAsiciStr(0, 110, 16, WHITE, BLACK, (uint8_t *)buf);

    sprintf(buf, "Tick = %lu", HAL_GetTick());
    LCD_WriteAsiciStr(0, 140, 16, WHITE, BLACK, (uint8_t *)buf);
}
//----------------------------------------T I M E 页 面 编 辑-----------------------------------------------------------------------------------
/* ---- TIME 页面 ---- */
static void time_draw(void)
{
    char buf[24];

    /* HAL_GetTick() 是开机毫秒数，除 1000 得到秒 */
    uint32_t sec = HAL_GetTick() / 1000;

    uint8_t hh = (uint8_t)((sec / 3600) % 100);   /* 时，超过 99 小时回绕 */
    uint8_t mm = (uint8_t)((sec % 3600) / 60);    /* 分 */
    uint8_t ss = (uint8_t)(sec % 60);             /* 秒 */

    /* %02d = 至少两位，不足补 0，所以 5 秒显示 "05" */
    sprintf(buf, "%02d:%02d:%02d", hh, mm, ss);

    /* 24 号字，比菜单正文大一号，看着像时钟 */
    LCD_WriteAsiciStr(20, 120, 24, WHITE, BLACK, (uint8_t *)buf);

    LCD_WriteAsiciStr(0, 290, 16, GRAY, BLACK, (uint8_t *)"long OK = back");
}
