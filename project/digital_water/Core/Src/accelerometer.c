#include "accelerometer.h"

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
		0x2D - write 0bx0[10]00[10] --> 0x22
		Configures low noise mode, turns off autosleep, and begins measurements

	*/

extern SPI_HandleTypeDef hspi3;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim6;
extern int overflow;

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
	print_msg("\n* Accelerometer initializing *\n\n");

	uint8_t device_id = accel_read(0x01);

	if (device_id != 0x1D)
	{

		// Read failed
		print_msg("Accelerometer initial read fail\n");
		sprintf(msg, "Read a value of: 0x%x\n", device_id);
		print_msg(msg);

	}
	
	// Soft reset
	accel_write(0x1F, 0x52);
	
	print_msg("Initializing accelerometer registers\n");
	
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET); // Set accelerometer CS low
	HAL_Delay(10);
	
	// Accelerometer register initialization via burst write
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
	tx_buff[15] = 0x22;	 // Power control
	
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi3, tx_buff, 16, 1000);
	
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_SET); // Set accelerometer CS high
	HAL_Delay(10);
	
	uint8_t read_val = accel_read(0x01);
	sprintf(msg, "Contents of register 0x01 is 0x%x\n", read_val);
	print_msg(msg);
	
	return HAL_OK;
}

HAL_StatusTypeDef accel_write(uint8_t reg, uint8_t val){
	
	char msg[100];
	sprintf(msg, "Writing 0x%x into register 0x%x\n", val, reg);
	print_msg(msg);
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET); // Set accelerometer CS low
	HAL_Delay(10);

	uint8_t tx_buff[3];
	uint8_t rx_buff[3];
	
	tx_buff[0] = 0x0A;
	tx_buff[1] = reg;
	tx_buff[2] = val;

	rx_buff[0] = 0x0A;
	rx_buff[1] = reg;
	rx_buff[2] = val;
	
	HAL_StatusTypeDef write_status;
	write_status = HAL_SPI_TransmitReceive(&hspi3, tx_buff, rx_buff, 3, 1000);
	
	if (write_status != HAL_OK) print_msg("Write no good\n");
	
	print_msg("tx_buff       rx_buff\n");
	for (int8_t i = 0; i < 3; i++) {
		sprintf(msg, "0x%x       0x%x\n", tx_buff[i], rx_buff[i]);
		print_msg(msg);
	}

	
	HAL_GPIO_WritePin(GPIOD, ACCEL_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(10);

	return write_status;
	
}


int8_t accel_read(int8_t reg)
{
	print_msg("Reading\n");

	char msg[100];
	
	uint8_t tx_buff[3];
	uint8_t rx_buff[3];
	uint8_t ret_val;

	tx_buff[0] = 0x0B; // Read instruction
	tx_buff[1] = reg;  // Register we want to read from
	tx_buff[2] = 0;	   // Dummy byte

	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET); // Set accelerometer CS low
	HAL_Delay(10);

	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi3, tx_buff, rx_buff, 3, 1000);

	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	
	if (status == HAL_OK)
		ret_val = rx_buff[2];
	else
		ret_val = -1;
	
	print_msg("tx_buff       rx_buff\n");
	for (int8_t i = 0; i < 3; i++) {
		sprintf(msg, "0x%x       0x%x\n", tx_buff[i], rx_buff[i]);
		print_msg(msg);
	}

	sprintf(msg, "Read on reg 0x%x returns value 0x%x\n", reg, ret_val);
	print_msg(msg);
	

	
	return ret_val;
}

HAL_StatusTypeDef accel_poll(int16_t *read_buff)
{

	char msg[100];
	// Burst reads to read all 6 registers for X, Y, Z accelerometer data

	uint8_t tx_buff[8];
	uint8_t rx_buff[8];

	tx_buff[0] = 0x0B; // Instruction to read register
	tx_buff[1] = 0x0E; // Corresponds to XDATA register
	for (int8_t i = 2; i < 8; i++)
		tx_buff[i] = 0; // indices 2, 3, 4 are dummy bytes
	
	HAL_GPIO_WritePin(ACCEL_CS_GPIO_Port, ACCEL_CS_Pin, GPIO_PIN_RESET); // Set accelerometer CS low
	//HAL_Delay(10);
	
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi3, tx_buff, rx_buff, 8, 1000);

	if (status == HAL_OK)
	{
		// in order: x, y, then z
		read_buff[0] = ((int16_t)rx_buff[3] << 8) | rx_buff[2]; // index 2 is the lower 8'b, then the next 
		read_buff[1] = ((int16_t)rx_buff[5] << 8) | rx_buff[4]; // index is the upper 4'b (sign extended)
		read_buff[2] = ((int16_t)rx_buff[7] << 8 )| rx_buff[6];
	}
	
	sprintf(msg, "\nX: %d\nY: %d\nZ: %d\n\n", rx_buff[2], rx_buff[3], rx_buff[4]);
	//print_msg(msg);
	
	HAL_GPIO_WritePin(GPIOD, ACCEL_CS_Pin, GPIO_PIN_SET);
	//HAL_Delay(10);

	return status;
}

/* Tried initializing registers with a burst write.

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
	
	HAL_StatusTypeDef write_status = HAL_SPI_TransmitReceive(&hspi3, tx_buff, rx_buff, 16, 1000);

*/
