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
};

//LED 灯的亮灭
static void Led1_On(void)   { HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET); }
static void Led1_Off(void)  { HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);   }
static void Led2_On(void)   { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); }
static void Led2_Off(void)  { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);   }

//定义顺序：被引用的必须先定义 所以需要从最深往外定义
//第三层
//1.LED1
static const menu_item_t LED1_item[]={
    {"on",NULL,Led1_On},
    {"off",NULL,Led1_Off}
};
static const menu_page_t led1_page={"LED1",LED1_item,2,two_y};

//2.LED2
static const menu_item_t LED2_item[]={
    {"on",NULL,Led2_On},
    {"off",NULL,Led2_Off}
};
static const menu_page_t led2_page={"LED2",LED2_item,2,two_y};
//第二层
static const menu_item_t LED_item[]={
    {"LED1",&led1_page,NULL},
    {"LED2",&led2_page,NULL}
};
static const menu_page_t LED_page={"LED",LED_item,2,two_y};
//第一层
static const menu_item_t main_items[] = {
    { "1.LED",   &LED_page,   NULL },
    { "2.PWM",   NULL,        NULL },   /* 还没做，确定后无反应 */
    { "3.TIME",  NULL,        NULL },
    { "4.INFO",  NULL,        NULL },
};
static const  menu_page_t main_page = { "Menu", main_items, 4, menu_y };


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
    cur_page=p;
    cur_sel=0;
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
