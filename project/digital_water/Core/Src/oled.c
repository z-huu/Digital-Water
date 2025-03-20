#include "oled.h"
// Resources
// https://digilent.com/reference/pmod/pmodoledrgb/reference-manual?redirect=1 -- Initialization commands
// https://digilent.com/reference/pmod/pmodoledrgb/start?redirect=1 -- Pinout

// TODO: need to configure three more GPIO outputs for DC, VCCEN, PMODEN

// DC:  High for Data, Low for Command -- PA4
// VCCEN: Pulldown PB3
// PMODEN: Pulldown PB5
// RST: Pulldown, PC7

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart3;

void my_print_msg(char *msg) {
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
}

HAL_StatusTypeDef oled_init(void) {
	
	// Set SPI3 CS low
	HAL_GPIO_WritePin(GPIOD, OLED_CS, GPIO_PIN_RESET); // Set accelerometer CS low
	
}

HAL_StatusTypeDef oled_write(uint8_t reg, uint8_t val) {
	
	uint8_t tx_buff[3];
	uint8_t rx_buff[3];
	
}