#include "oled.h"
// Resources
// https://digilent.com/reference/pmod/pmodoledrgb/reference-manual?redirect=1 -- Initialization commands
// https://digilent.com/reference/pmod/pmodoledrgb/start?redirect=1 -- Pinout

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
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); // Bring CS pin low
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_RESET); // Bring DC pin low
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);   // Bring RST pin high
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_RESET); // Bring VCCEN pin low
	HAL_GPIO_WritePin(OLED_PMODEN_GPIO_Port, OLED_PMODEN_Pin, GPIO_PIN_SET); // Bring PMODEN pin high
	
	HAL_Delay(20); // delay 20 ms to allow the 3.3V rail to become stable.
	
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); // Bring RST low
	
	HAL_Delay(10); // wait for at least 3 us
	
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET); // Bring RST back high to reset display controller\
	
	HAL_Delay(10); // wait for at least 3 us for reset operation to complete

	// Enable the driver IC to accept commands by sending 0xFD, 0x12
	HAL_StatusTypeDef write_status = oled_write(0xFD);
	write_status |= oled_write(0x12);
	if (write_status != HAL_OK) my_print_msg("ENABLECMD write bad\n");
	
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_SET); // Bring CS pin high
	HAL_Delay(10);

	// Various OLED configurations
	
  write_status |= oled_cmd(CMD_DISPLAY_OFF);          //Display Off
  write_status |= oled_cmd(CMD_SET_CONTRAST_A);       //Set contrast for color A
  write_status |= oled_cmd(0x91);                     //145 (0x91)
  write_status |= oled_cmd(CMD_SET_CONTRAST_B);       //Set contrast for color B
  write_status |= oled_cmd(0x50);                     //80 (0x50)
  write_status |= oled_cmd(CMD_SET_CONTRAST_C);       //Set contrast for color C
  write_status |= oled_cmd(0x7D);                     //125 (0x7D)
  write_status |= oled_cmd(CMD_MASTER_CURRENT_CONTROL);//master current control
  write_status |= oled_cmd(0x06);                     //6
  write_status |= oled_cmd(CMD_SET_PRECHARGE_SPEED_A);//Set Second Pre-change Speed For ColorA
  write_status |= oled_cmd(0x64);                     //100
  write_status |= oled_cmd(CMD_SET_PRECHARGE_SPEED_B);//Set Second Pre-change Speed For ColorB
  write_status |= oled_cmd(0x78);                     //120
  write_status |= oled_cmd(CMD_SET_PRECHARGE_SPEED_C);//Set Second Pre-change Speed For ColorC
  write_status |= oled_cmd(0x64);                     //100
  write_status |= oled_cmd(CMD_SET_REMAP);            //set remap & data format
  write_status |= oled_cmd(0x72);                     //0x72
  write_status |= oled_cmd(CMD_SET_DISPLAY_START_LINE);//Set display Start Line
  write_status |= oled_cmd(0x0);
  write_status |= oled_cmd(CMD_SET_DISPLAY_OFFSET);   //Set display offset
  write_status |= oled_cmd(0x0);
  write_status |= oled_cmd(CMD_NORMAL_DISPLAY);       //Set display mode
  write_status |= oled_cmd(CMD_SET_MULTIPLEX_RATIO);  //Set multiplex ratio
  write_status |= oled_cmd(0x3F);
  write_status |= oled_cmd(CMD_SET_MASTER_CONFIGURE); //Set master configuration
  write_status |= oled_cmd(0x8E);
  write_status |= oled_cmd(CMD_POWER_SAVE_MODE);      //Set Power Save Mode
  write_status |= oled_cmd(0x00);                     //0x00
  write_status |= oled_cmd(CMD_PHASE_PERIOD_ADJUSTMENT);//phase 1 and 2 period adjustment
  write_status |= oled_cmd(0x31);                     //0x31
  write_status |= oled_cmd(CMD_DISPLAY_CLOCK_DIV);    //display clock divider/oscillator frequency
  write_status |= oled_cmd(0xF0);
  write_status |= oled_cmd(CMD_SET_PRECHARGE_VOLTAGE);//Set Pre-Change Level
  write_status |= oled_cmd(0x3A);
  write_status |= oled_cmd(CMD_SET_V_VOLTAGE);        //Set vcomH
  write_status |= oled_cmd(0x3E);
  write_status |= oled_cmd(CMD_DEACTIVE_SCROLLING);   //disable scrolling
  write_status |= oled_cmd(CMD_NORMAL_BRIGHTNESS_DISPLAY_ON);//set display on
	
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_SET); // Bring VCCEN pin high
	HAL_Delay(25); // then wait for at least 25 ms

	if (write_status != HAL_OK) my_print_msg("Bad init\n");

	// Turn display on
	write_status = oled_cmd(CMD_NORMAL_BRIGHTNESS_DISPLAY_ON);
	if (write_status != HAL_OK) my_print_msg("ON write bad\n");
	
	// Wait 100 ms before using the display
	HAL_Delay(100);

	my_print_msg("\n* OLED initialization complete *\n\n");
	
	return write_status; 
}

HAL_StatusTypeDef oled_write(uint8_t val) {
	// CS and DC pins already managed outside of this function
	
 	uint8_t tx_buff[1];
	tx_buff[0] = val;
	//while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	HAL_StatusTypeDef write_status = HAL_SPI_Transmit(&hspi1, tx_buff, 1, 1000);
	return write_status;
}

HAL_StatusTypeDef oled_data(uint8_t data){
	
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_SET); // Set OLED DC high, since sending data
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); // Set OLED cs low
	//HAL_Delay(10);
	
	HAL_StatusTypeDef stat = oled_write(data);
	
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET); // Set OLED cs high again to disable

	//HAL_Delay(10);
	return stat;
}

HAL_StatusTypeDef oled_cmd(uint8_t cmd) {
	
	HAL_GPIO_WritePin(OLED_DCL_GPIO_Port, OLED_DCL_Pin, GPIO_PIN_RESET); // Set OLED DC low, since sending cmd
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET); // Set OLED cs low
	HAL_Delay(1);
	
	HAL_StatusTypeDef stat = oled_write(cmd);
	
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET); // Set OLED cs high again to disable
	//HAL_Delay(1);
	return stat;
}



HAL_StatusTypeDef oled_off(void) {
	
	HAL_StatusTypeDef write_status = oled_write(0xAE);
	if (write_status != HAL_OK) my_print_msg("Bad write in OLED off.\n");
	
  // Bring VCCEN pin low
	HAL_GPIO_WritePin(OLED_VCCEN_GPIO_Port, OLED_VCCEN_Pin, GPIO_PIN_RESET);
	HAL_Delay(400); // Delay for 400 ms
	
	// From this point on, safe to remove power to OLED.
	
	return write_status;
	
}

HAL_StatusTypeDef oled_drawpixel(uint8_t col, uint8_t row, uint16_t color){
	
  //set column point
	HAL_StatusTypeDef stat;
  stat |= oled_cmd(CMD_SET_COLUMN_ADDRESS);
  stat |= oled_cmd(col);
  stat |= oled_cmd(RGB_OLED_WIDTH-1);
  //set row point
  stat |= oled_cmd(CMD_SET_ROW_ADDRESS);
  stat |= oled_cmd(row);
  stat |= oled_cmd(RGB_OLED_HEIGHT-1);
	
	oled_data(color >> 8);
	oled_data(color);
	
	return stat;
}

void oled_drawline(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
	
 if((x0 < 0) || (y0 < 0) || (x1 < 0) || (y1 < 0)) return;

 if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
 if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
 if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
 if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;

 oled_cmd(CMD_DRAW_LINE); 
 oled_cmd(x0);						//start column
 oled_cmd(y0);						//start row
 oled_cmd(x1);						//end column
 oled_cmd(y1);						//end row
 oled_cmd((uint8_t)((color>>11)&0x1F));//R
 oled_cmd((uint8_t)((color>>5)&0x3F));//G
 oled_cmd((uint8_t)(color&0x1F));//B
}

void oled_eraseRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	
	oled_cmd(CMD_CLEAR_WINDOW);
	oled_cmd(x0);
	oled_cmd(y0);
	oled_cmd(x1);
	oled_cmd(y1);
}

void oled_drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t border_col, uint16_t fill_col) {
	
	if (x0 >= RGB_OLED_WIDTH)  x0 = RGB_OLED_WIDTH - 1;
  if (y0 >= RGB_OLED_HEIGHT) y0 = RGB_OLED_HEIGHT - 1;
  if (x1 >= RGB_OLED_WIDTH)  x1 = RGB_OLED_WIDTH - 1;
  if (y1 >= RGB_OLED_HEIGHT) y1 = RGB_OLED_HEIGHT - 1;
	
	oled_cmd(CMD_FILL_WINDOW);//fill window
	oled_cmd(ENABLE_FILL);
	oled_cmd(CMD_DRAW_RECTANGLE);//draw rectangle
	oled_cmd(x0);//start column
	oled_cmd(y0);//start row
	oled_cmd(x1);//end column
	oled_cmd(y1);//end row
	oled_cmd((uint8_t)((border_col>>11)&0x1F));//R
	oled_cmd((uint8_t)((border_col>>5)&0x3F));//G
	oled_cmd((uint8_t)(border_col&0x1F));//B
	oled_cmd((uint8_t)((fill_col>>11)&0x1F));//R
	oled_cmd((uint8_t)((fill_col>>5)&0x3F));//G
	oled_cmd((uint8_t)(fill_col&0x1F));//B
	
}

