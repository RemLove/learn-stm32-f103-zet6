#include "Menu.h"

/* Menu tree:
 *   MENU (folder)
 *     +-- SET (folder)
 *     |     +-- VOLUME (leaf)
 *     |     +-- VAL     (leaf, brightness)
 *     +-- ABOUT (leaf)
 *     +-- STEP  (leaf)
 */
Node_t Menu[] = {
  [0] = {"MENU",   NODE_FOLDER, {1, 2, 3}, 3, NULL},  /* menu root   */
  [1] = {"SET",    NODE_FOLDER, {4, 5},    2, NULL},  /* settings    */
  [2] = {"ABOUT",  NODE_LEAF,   {0},        0, NULL},  /* about       */
  [3] = {"STEP",   NODE_LEAF,   {0},        0, NULL},  /* pedometer   */
  [4] = {"VOLUME", NODE_LEAF,   {0},        0, NULL},  /* volume      */
  [5] = {"VAL",    NODE_LEAF,   {0},        0, NULL},  /* brightness  */
};

int path[10] = {0};  /* stack of node IDs along current path */
int cur = 0;         /* index of selected child within current folder */
int top = 0;         /* stack depth (number of nodes in path) */

void Menu_Init(void)
{
    path[0] = 0;     /* root node ID = 0 */
    cur = 0;
    top = 1;
}

int here(void)
{
    return path[top - 1];  /* current folder node ID */
}

/* move selection up */
void Key_Up(void)
{
    int n = Menu[here()].childrenNum;
    cur = (cur - 1 + n) % n;
}

/* move selection down */
void Key_Down(void)
{
    int n = Menu[here()].childrenNum;
    cur = (cur + 1) % n;
}

void Key_Enter(void)
{
    if (Menu[here()].type == NODE_FOLDER)
    {
        /* enter selected child folder */
        path[top++] = Menu[here()].childId[cur];
        cur = 0;
    }
    else if (Menu[here()].type == NODE_LEAF)
    {
        /* execute leaf action if any */
        if (Menu[here()].action != NULL)
        {
            Menu[here()].action();
        }
    }
}

void Key_Back(void)
{
    if (top > 1)
    {
        /* go back to parent folder */
        top--;
        cur = 0;
    }
}

void Menu_Display(void)
{
    uint8_t i;
    uint8_t n = Menu[here()].childrenNum;

    OLED_Clear();
    OLED_ShowString(0, 0, Menu[here()].name, OLED_8X16);
    for (i = 0; i < n; i++)
    {
        if (i == cur)
        {
            OLED_ShowString(0, 16 + i * 16, ">", OLED_8X16);  /* selection marker */
        }
        OLED_ShowString(8, 16 + i * 16, Menu[Menu[here()].childId[i]].name, OLED_8X16);
    }
    OLED_Update();
}
