#ifndef __BOARD_KEY_H__
#define __BOARD_KEY_H__

#include "stm32f4xx.h"

typedef enum {
    NOKEY=0,
    KEY1=1,
    KEY2,
    KEY3,
}KEY_enum;


void board_key_init(void);
KEY_enum board_key_scan(void);

#endif
