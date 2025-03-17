#include "fluid_sim.h"
#include "physics.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// FLUID SIM Initializations
Sim_Cell_t grid_array[SIM_PHYS_X_SIZE][SIM_PHYS_Y_SIZE];

void Sim_Grid_Init(){
    // set bounding box to be a solid
    for(int i = 0; i < SIM_PHYS_X_SIZE; i++){
        for(int k = 0; k < SIM_PHYS_Y_SIZE; k++){
            if(i == 0 || k == 0 || i == SIM_PHYS_X_SIZE - 1 || k == SIM_PHYS_Y_SIZE - 1){

            }
        }
    }
}

void Sim_Grid_Update() { 
    float deltaTime = 1 / SIM_PHYSICS_FPS; 

    for(int i = 0; i < SIM_PHYS_X_SIZE; i++){
        for(int k = 0; k < SIM_PHYS_Y_SIZE; k++){
            grid_array[i][k].velocity.y += deltaTime * SIM_GRAV
        }
    }
}

// FOR SERIAL MONITOR USE:
extern uint8_t tx_buff[sizeof(PREAMBLE) +
                       SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE + sizeof(SUFFIX)];
extern size_t tx_buff_len;
extern UART_HandleTypeDef huart3;

void testPrint(void) {
  // Create a new buffer from the snapshot_buffer than the DCMI copied the
  // 16-bit pixel values into.
  tx_buff_len = 0;

  // Add the START preamble message to the start of the buffer for the
  // serial-monitor program.
  for (int i = 0; i < sizeof(PREAMBLE); i++) {
    tx_buff[i] = PREAMBLE[i];
  }

  // Write code to copy every other byte from the main frame buffer to
  // our temporary buffer (this converts the image to grey scale)

  tx_buff_len = sizeof(PREAMBLE);

  for (int i = 0; i < SIM_X_SIZE * SIM_Y_SIZE; i++) {
    // if (i % 2 == 0) continue;
    char pixel = (i / SIM_X_SIZE);
    tx_buff[tx_buff_len++] = pixel;
    // print_msg(&pixel);
  }

  // Load the END suffix message to the end of the message.
  for (int i = 0; i < sizeof(SUFFIX); i++) {
    tx_buff[tx_buff_len++] = SUFFIX[i];
  }

  while (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY) {
  }
  HAL_UART_Transmit_DMA(&huart3, (uint8_t *)tx_buff,
                        sizeof(PREAMBLE) + SIM_X_SIZE * SIM_Y_SIZE);
}
