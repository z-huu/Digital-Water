#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include <stdio.h>
#include <string.h>

void my_print_msg(char *msg);

HAL_StatusTypeDef oled_init(void);

#endif