#include "accelerometer.h"
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart3;

void my_print_msg(char *msg) {
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
}
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

HAL_StatusTypeDef accel_init(void)
{
	
	
	char msg[100];
	my_print_msg("Accelerometer initializing\n");
	
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_RESET); // Set accelerometer CS low
	

	int8_t device_id = accel_read(0x01);
	if (device_id != 0x1D)
	{

		// Read failed
		my_print_msg("Accelerometer initial read fail\n");
		sprintf(msg, "Read a value of: 0x%x\n", device_id);
		while (1)
			;
	}

	/* Burst write to initialize registers. Writing to registers 0x20 to 0x2D

	Activity Threshold registers -- if any accelerometer data exceeds this, seen as Activity Event
		0x20 - lower 8 bits of Activity Threshold value
		0x21 - upper 3 bits of Activity Threshold value; 0x21 register's upper 5 bits are UNUSED

	Activity Time register -- num. consecutive samples that exceed activity theshold required before
	causing an Activity Event
		0x22 - 8'b value

	Inactivity Threshold registers -- if any accelerometer data is below this, seen as Inactivity Event
		0x23 - lower 8 bits of Inactivity Threshold value
		0x24 - upper 3 bits of Inactivity Threshold value

	Inactivity Time registers -- num. consecutive samples lower than inactivity thresold required before
	causing an Inactivity Event
		0x25 - lower 8 bits
		0x26 - upper 8 bits

	Activity/Inactivity Control : Configure Activity and Inactivity events. Upper 2 bits are unused
		0x27 - write 0b00[11]1011 --> 0x3B
		Enables Activity events in "referenced" mode.
		Activity interrupts do not need to be acknowledged by the STM32.

	FIFO Control : Configure FIFO sample ranges and operating mode. Upper 4 bits unused
		0x28 - write 0bxxxx00[00] --> 0x00
		Disables FIFO.

	FIFO Samples : configure num. samples to store in FIFO
		0x29 - write 0x00
		Store 0 samples in FIFO.

	INT1MAP : Configure accelerometer pin interrupt 1
		0x2A -  write 0b00010000 --> 0x10
		Maps INT1 pin to the accelerometer's Activity status

	INT2MAP : Configure accelerometer pin interrupt 2
		0x2B - write 0x00
		Currently not using INT2.

	Filter Control : Configure accelerometer measurement range, frequency, and filtering
		0x2C - write 0b[00]x10[011] --> 0x13
		Configures measurements at +- 2g, antialiasing bandwidth at 1/4 of output data rate,
		and output data rate at 100 Hz

	Power Control : Configure power mode, noise filtering, and start / stop
		0x2D - write 0bx0[01]00[10] --> 0x12
		Configures low noise mode, turns off autosleep, and begins measurements

	*/

	my_print_msg("Initializing accelerometer registers\n");
	
	uint8_t tx_buff[16];

	tx_buff[0] = 0x0A; // Write instruction
	tx_buff[1] = 0x20; // Address of first register
	tx_buff[2] = 0x64; // Lower 8'b of Activity Threshold (100 mg threshold)
	tx_buff[3] = 0;	   // Upper 3'b of Activity Threshold
	tx_buff[4] = 0;	   // Activity time (8'b)
	tx_buff[5] = 0;    // Lower 8'b of Inactivity Threshold
	tx_buff[6] = 0;		 // Upper 3'b of Inactivity Threshold
	tx_buff[7] = 0;		 // Lower 8'b of Inactivity Time
	tx_buff[8] = 0;		 // Upper 8'b of Inactivity Time
	tx_buff[9] = 0x3B; // Activity/Inactivity Control Register
	tx_buff[10] = 0;	 // FIFO control
	tx_buff[11] = 0;	 // FIFO samples
	tx_buff[12] = 0x10;// INT1 configuration
	tx_buff[13] = 0;	 // INT2 configuration
	tx_buff[14] = 0x13;	 // Filter control
	tx_buff[15] = 0x12;	 // Power control
	
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 16, 1000);
	
	if (write_status != HAL_OK) {
		// Initialization writes failed
		my_print_msg("Accelerometer register initializations fail\n");
		while(1);
	}
	
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_SET); // Set accelerometer CS high

	return HAL_OK;
}

int8_t accel_read(int8_t reg)
{

	uint8_t tx_buff[3];
	uint8_t rx_buff[3];
	int8_t ret_val;

	tx_buff[0] = 0x0B; // Read instruction
	tx_buff[1] = reg;  // Register we want to read from
	tx_buff[2] = 0;	   // Dummy byte

	// Pull SPI1 CS low
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_RESET);

	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_buff, rx_buff, 3, 1000);

	if (status == HAL_OK)
		ret_val = rx_buff[2];
	else
		ret_val = -1;

	// Set SP1 CS high again
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_SET);
	return ret_val;
}

HAL_StatusTypeDef accel_poll(uint8_t *read_buff)
{

	// Read from XDATA and then do burst reads to grab YDATA and ZDATA
	// Pull SPI1 CS low
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_RESET);

	uint8_t tx_buff[5];
	uint8_t rx_buff[5];

	tx_buff[0] = 0x0B; // Instruction to read register
	tx_buff[1] = 0x08; // Corresponds to XDATA register
	for (int8_t i = 2; i < 5; i++)
		tx_buff[i] = 0; // indices 2, 3, 4 are dummy bytes

	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, tx_buff, rx_buff, 5, 1000);

	if (status == HAL_OK)
	{

		read_buff[0] = rx_buff[2]; // Indices 0 and 1 of rx_buff are dummy bytes.
		read_buff[1] = rx_buff[3]; // Actual data is in indices 2, 3, 4
		read_buff[2] = rx_buff[4];
	}

	// Set SP1 CS high again
	HAL_GPIO_WritePin(GPIOC, SPI1_CS_Pin, GPIO_PIN_SET);
	return status;
}
