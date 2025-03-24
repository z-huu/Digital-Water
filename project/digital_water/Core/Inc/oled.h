#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdio.h>
#include <string.h>

HAL_StatusTypeDef oled_init(void);
HAL_StatusTypeDef oled_off(void);
HAL_StatusTypeDef oled_drawpixel(uint8_t col, uint8_t row, uint16_t color);

#endif