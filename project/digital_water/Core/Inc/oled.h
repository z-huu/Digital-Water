#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdio.h>
#include <string.h>

void my_print_msg(char *msg);

HAL_StatusTypeDef oled_init(void);
HAL_StatusTypeDef oled_write(uint8_t reg, uint8_t val);

#endif