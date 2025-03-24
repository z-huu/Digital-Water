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

HAL_StatusTypeDef oled_init(void) {
	
	char msg[100];
	my_print_msg("\n* Initializing OLED *\n\n");
	// Power Up Sequence
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_RESET); // Bring DC pin low
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);   // Bring RST pin high
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_RESET); // Bring VCCEN pin low
	HAL_GPIO_WritePin(OLED_PMODEN_GPIO_Port, OLED_PMODEN_Pin, GPIO_PIN_SET); // Bring PMODEN pin high
	HAL_Delay(20); // delay 20 ms to allow the 3.3V rail to become stable.
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // Bring RST low
	HAL_Delay(1); // wait for at least 3 us
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET); // Bring RST back high to reset display controller
	HAL_Delay(1); // wait for at least 3 us for reset poperation to complete

	// Enable the driver IC to accept commands by sending 0xFD, 0x12
	uint8_t tx_buff[2];
	tx_buff[0] = 0xFD;
	tx_buff[1] = 0x12;
	
	// Set OLED CS low
	HAL_GPIO_WritePin(GPIOD, OLED_CS_Pin, GPIO_PIN_RESET); 
	HAL_Delay(1);
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000);
	if (write_status != HAL_OK) my_print_msg("ENABLECMD write bad\n");

	// Send command to turn display off
	tx_buff[0] = 0xAE;
	tx_buff[1] = 0;
	write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000);
	if (write_status != HAL_OK) my_print_msg("DISPOFF write bad\n");
	
	// Send commands to set Remap and Display formats
	tx_buff[0] = 0xA0;
	tx_buff[1] = 0x72;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("REMAP write bad\n");
	
	// Send command to place display start line @ top line
	tx_buff[0] = 0xA1;
	tx_buff[1] = 0;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("STARTLINE write bad\n");

	// Send command to configure no vertical display offset
	tx_buff[0] = 0xA2;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("VOFFSET write bad\n");

	// Set default settings
	tx_buff[0] = 0xA4;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000)) != HAL_OK) my_print_msg("DEFAULT write bad\n");
	
	// Set multiplex ratio
	tx_buff[0] = 0xA8;
	tx_buff[1] = 0x3F;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("MULTIPLEX write bad\n");

	// Configure display to use external VCC supply
	tx_buff[0] = 0xAD;
	tx_buff[1] = 0x8E;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("EXTVCC write bad\n");
	
	// Disable power saving mode
	tx_buff[0] = 0xB0;
	tx_buff[1] = 0x0B;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PWRSAVE write bad\n");
	
	// Set phase length
	tx_buff[0] = 0xB1;
	tx_buff[1] = 0x31;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PHASE write bad\n");

	// Set clock divide ratio
	tx_buff[0] = 0xB3;
	tx_buff[1] = 0xF0;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("CLKDIV write bad\n");
	
	// Set COLOR A pre-charge speed
	tx_buff[0] = 0x8A;
	tx_buff[1] = 0x64;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PRECHARGE1 write bad\n");
	
	// Set COLOR B pre-charge speed
	tx_buff[0] = 0x8B;
	tx_buff[1] = 0x78;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PRECHARGE2 write bad\n");
	
	// Set COLOR C pre-charge speed
	tx_buff[0] = 0x8C;
	tx_buff[1] = 0x64;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PRECHARGE3 write bad\n");

	// Set pre-charge voltage
	tx_buff[0] = 0xBB;
	tx_buff[1] = 0x3A;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("PRECHARGEV write bad\n");

	// Set VCOMH
	tx_buff[0] = 0xBE;
	tx_buff[1] = 0x3E;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("VCOMH write bad\n");

	// Set attenuation factor
	tx_buff[0] = 0x87;
	tx_buff[1] = 0x06;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("ATTENUATION write bad\n");
	
	// Set COLOR A contrast
	tx_buff[0] = 0x81;
	tx_buff[1] = 0x91;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("CONTRAST1 write bad\n");

	// Set COLOR B contrast
	tx_buff[0] = 0x82;
	tx_buff[1] = 0x50;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("CONTRAST2 write bad\n");
	
	// Set COLOR C contrast
	tx_buff[0] = 0x83;
	tx_buff[1] = 0x7D;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 2, 1000)) != HAL_OK) my_print_msg("CONTRAST3 write bad\n");
	
	// Disable scrolling
	tx_buff[0] = 0x2E;
	tx_buff[1] = 0;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000)) != HAL_OK) my_print_msg("SCROLLING write bad\n");
	
	// Clear the screen
	uint8_t new_tx_buff[5];
	new_tx_buff[0] = 0x25;
	new_tx_buff[1] = 0;
	new_tx_buff[2] = 0;
	new_tx_buff[3] = 0x5F;
	new_tx_buff[4] = 0x3F;
	if ((HAL_SPI_Transmit(&hspi1, new_tx_buff, 5, 1000)) != HAL_OK) my_print_msg("CLEAR write bad\n");
	
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_SET); // Bring VCCEN pin high
	HAL_Delay(25); // then wait for at least 25 ms


	// Turn display on
	tx_buff[0] = 0xAE;
	if ((HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000)) != HAL_OK) my_print_msg("ON write bad\n");

	// Wait 100 ms before using the display
	HAL_Delay(100);

	my_print_msg("\n* OLED initialization complete *\n\n");
	
	HAL_GPIO_WritePin(GPIOD, OLED_CS_Pin, GPIO_PIN_SET); // Set OLED cs high again to disable

	return write_status; // probably add some better logic for this; write_status |= each write
}

HAL_StatusTypeDef oled_off(void) {
	
	uint8_t tx_buff[1] = {0xAE};
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000);
	if (write_status != HAL_OK) my_print_msg("Bad write in OLED off.\n");
	
  // Bring VCCEN pin low
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_RESET);
	HAL_Delay(400); // Delay for 400 ms
	
	// From this point on, safe to remove power to OLED.
	
	return write_status;
	
}

HAL_StatusTypeDef oled_drawpixel(uint8_t col, uint8_t row, uint16_t color){
	
	// Should DC be high or low for this transmission?
	// Command to draw rectangle : 0x22
	/*
	uint8_t ExtractRFromRGB(uint16_t wRGB){return (uint8_t)((wRGB>>11)&0x1F);};
	uint8_t ExtractGFromRGB(uint16_t wRGB){return (uint8_t)((wRGB>>5)&0x3F);};	
	uint8_t ExtractBFromRGB(uint16_t wRGB){return (uint8_t)(wRGB&0x1F);};
	
	*/
	
	uint8_t tx_buff[11];
	tx_buff[0] = 0x22; // Command to draw rectangle.
	tx_buff[1] = col; // Start col
	tx_buff[2] = row; // Start row
	tx_buff[3] = col; // End col
	tx_buff[4] = row; // End row
	// Outline colors
	tx_buff[5] = (uint8_t)((color >> 11)&0x1F); // Extracting R bits from RGB
	tx_buff[6] = (uint8_t)((color >> 5)&0x3F);
	tx_buff[7] = (uint8_t)(color&0x1F);
	// Fill colors
	tx_buff[8] = (uint8_t)((color >> 11)&0x1F); // Extracting R bits from RGB
	tx_buff[9] = (uint8_t)((color >> 5)&0x3F);
	tx_buff[10] =(uint8_t)(color&0x1F);
	
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_RESET); // Bring DC low because we're writing commands
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); // Bring OLED CS low
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 11, 1000);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET); // Bring OLED CS back high

	if (write_status != HAL_OK) my_print_msg("draw_pixel write bad\n");
	
	return write_status;
}
