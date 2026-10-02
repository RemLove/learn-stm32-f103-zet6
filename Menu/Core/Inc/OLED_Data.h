#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/* Charset select: only ONE of the following two macros may be enabled */
#define OLED_CHARSET_UTF8      /* use UTF-8 charset */
/* #define OLED_CHARSET_GB2312 */  /* use GB2312 charset */

/* Chinese glyph cell */
typedef struct
{

#ifdef OLED_CHARSET_UTF8
    char Index[5];              /* UTF-8 char index, 5 bytes */
#endif

#ifdef OLED_CHARSET_GB2312
    char Index[3];              /* GB2312 char index, 3 bytes */
#endif

    uint8_t Data[32];           /* glyph bitmap data */
} ChineseCell_t;

/* ASCII font bitmap */
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];

/* Chinese font bitmap */
extern const ChineseCell_t OLED_CF16x16[];

/* image data */
extern const uint8_t Diode[];

#endif
