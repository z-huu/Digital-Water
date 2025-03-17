#include "accelerometer.h"
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi1;

// Everything you need to write this library is in these documents
// https://www.analog.com/media/en/technical-documentation/data-sheets/adxl362.pdf
// https://digilent.com/reference/pmod/pmodacl2/reference-manual
// https://stackoverflow.com/questions/67922914/stm32-spi-communication-with-hal

/* Accelerometer Instructions

	0x0A: write register
	0x0B: read register
	0x0D: read FIFO

	 Accelerometer Registers (8'b)
	 
	These contain 8 MSB of accelerometer data. There is an alternative that provides data with
	more resolution, but they require two reads and I'm unsure if we need the extra resolution.
	0x08: XDATA
	0x09: YDATA
	0x10: ZDATA
	
	STATUS Register. The LSB is the Data Ready bit, which is high if there is a new sample available
	0x0B: ERR_USER_REGS, AWAKE, INACT, ACT, FIFO_OVERRUN, FIFO_WATERMARK, FIFO_READY, DATA_READY
	
	0x01: Device ID register. Contains 0x1D
*/
// HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
// HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)

HAL_StatusTypeDef accel_init(void){
	
	int8_t device_id = accel_read(0x01);
	if (device_id != 0x1D) {
		
		// Read failed
		while(1);
	}
	
	return HAL_OK;
	
}


int8_t accel_read(int8_t reg){
	
	uint8_t tx_buff[3];
  uint8_t rx_buff[3];
	int8_t ret_val;
	
	tx_buff[0] = 0x0B; // Read instruction
	tx_buff[1] = reg; // Register we want to read from
	tx_buff[2] = 0; // Dummy byte
	
	// Pull SPI1 CS low
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_RESET);
	
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_buff, rx_buff, 3, 1000);

	if (status == HAL_OK) ret_val = rx_buff[2];
	else ret_val = -1;
	
	// Set SP1 CS high again
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_SET);
	return ret_val;
}

HAL_StatusTypeDef accel_poll(uint8_t *read_buff){
	
	// Read from XDATA and then do burst reads to grab YDATA and ZDATA
	// Pull SPI1 CS low
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_RESET);
	
	uint8_t tx_buff[5];
  uint8_t rx_buff[5];
	
	tx_buff[0] = 0x0B; // Instruction to read register
	tx_buff[1] = 0x08; // Corresponds to XDATA register
	for (int8_t i = 2; i < 5; i++) tx_buff[i] = 0; // indices 2, 3, 4 are dummy bytes
	
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_buff, rx_buff, 5, 1000);
	
	if (status == HAL_OK) {
		
		read_buff[0] = rx_buff[2]; // Indices 0 and 1 of rx_buff are dummy bytes. 
		read_buff[1] = rx_buff[3]; // Actual data is in indices 2, 3, 4
		read_buff[2] = rx_buff[4];

	}
	
	// Set SP1 CS high again
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_SET);
	return status;
}
