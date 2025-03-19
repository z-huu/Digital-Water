#include "oled.h"

extern SPI_HandleTypeDef hspi3;
extern UART_HandleTypeDef huart3;

void my_print_msg(char *msg) {
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
}