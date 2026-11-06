#include "web.h"

uint8_t tx_buf[2048];
uint8_t rx_buf[2048];
uint8_t cnt = 8;
uint8_t socklist[8] = {0, 1, 2, 3, 4, 5, 6, 7};

uint8_t content[] = "<!doctype html>\n"
                    "<html lang=\"en\">\n"
                    "<head>\n"
                    "   <meta charset=\"GBK\">\n"
                    "   <meta name=\"viewport\"\n"
                    "           content=\"width=device-width, user-scalable=no, initial-scale=1.0\">\n"   // 行21截图只拍到 "...no, i"，此处按标准 viewport 写法补全
                    "   <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n"
                    "   <title>尚硅谷嵌入式课程</title>\n"
                    "\n"
                    "   <style type=\"text/css\">\n"
                    "       #open_red{\n"
                    "           color: red;\n"
                    "           width: 100px;\n"
                    "           height: 40px;\n"
                    "\n"
                    "\n"
                    "       }\n"
                    "       #close_red{\n"
                    "           color: black;\n"
                    "           width: 100px;\n"
                    "           height: 40px;\n"
                    "       }\n"
                    "   </style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<a href=\"/index.html?action=1\"><button id=\"open_red\" >/*开*/</button></a>\n"
                    "<a href=\"/index.html?action=2\"><button id=\"close_red\" >/*关*/</button></a>\n"
                    "<a href=\"/index.html?action=3\"><button id=\"close_red\" >/*按钮文字在视口外*/</button></a>\n"
                    "</body>\n"
                    "</html>";


void Web_Init(void)
{
    //1.初始化
    LED_Init();
    httpServer_init(tx_buf, rx_buf, cnt, socklist);
    //2.注册http页面 表示服务器要响应的内容
    reg_httpServer_webContent((uint8_t *)"index.html",content);

}

void Web_Start(void)
{
    for (int i = 0; i < cnt; i++)
    {
        httpServer_start(i);
    }
}

void handle_web_func(uint8_t * url)
{
    //1.提取url中的action参数
    uint8_t * action = strstr((const char *)url, "action=");
    //2.根据action参数的值来控制LED灯的状态
    do_led_action(action);  
}
