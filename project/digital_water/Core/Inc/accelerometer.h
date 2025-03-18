#ifndef __ACCELEROMETER_H
#define __ACCELEROMETER_H

#include "main.h"
#include <stdio.h>
#include <string.h>

// Accelerometer will be connected on SPI 1 line: pin PC4
#define ACCEL_CS_PIN SPI1_CS_PIN
#define ACCEL_CS_GPIO_PORT SPI1_CS_GPIO_PORT
//SPI1_CS_GPIO_PORT from main.h

void my_print_msg(char *msg);

// VDD pins on STM32F446 are 3.3V, so should be compatible with accelerometer.
HAL_StatusTypeDef accel_init(void);
int8_t accel_read (int8_t reg);
HAL_StatusTypeDef accel_write(int8_t reg, int8_t val);
HAL_StatusTypeDef accel_poll(uint8_t *read_buff);

#endif