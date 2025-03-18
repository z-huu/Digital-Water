#include "fluid_sim.h"
#include "physics.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

// FLUID SIM Initializations
/*
Notes:
Heavily Based on the work done by Matthias Müller and his YouTube channel "Ten
Minute Physics"
*/
extern Sim_Cell_t grid_array[SIM_PHYS_X_SIZE][SIM_PHYS_Y_SIZE];
extern Sim_Particle_t particle_array[SIM_PARTICLE_COUNT];
extern Sim_Particle_t obstacle_array[SIM_OBSTACLE_COUNT];

extern int sim_time;

/*
Much of the simulation ideas are heavily based on the TenMinutePhysics code
Key difference is this is adapted from JavaScript into C code, with minor
modifications


*/
void Sim_Particle_HandleObstacleCollisions(Sim_Particle_t obstacle) {
  // simply check if in radius
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    Sim_Particle_t currentParticle = particle_array[k];

    float dx = currentParticle.position.x - obstacle.position.x;
    float dy = currentParticle.position.y - obstacle.position.y;
    float dxy_squared = dx * dx + dy * dy;

    float min_distance = obstacle.radius + currentParticle.radius;
    float min_dist_squared = min_distance * min_distance;
    if (dxy_squared < min_dist_squared) {
      // collision, simply inherit velocity
      currentParticle.velocity.x = obstacle.velocity.x;
      currentParticle.velocity.y = obstacle.velocity.y;
    }
  }
}

void Sim_Particle_HandleCellCollisions() {
  // for each particle...
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    // check boundary conditions
    Sim_Particle_t currentParticle = particle_array[k];
    Sim_Cell_t *currentCell = GetCellFromPosition(currentParticle.position);

    if (currentParticle.position.x < 0) {
      currentParticle.position.x = 0;
      currentParticle.velocity.x = 0;
    }
    if (currentParticle.position.x > SIM_PHYS_X_SIZE) {
      currentParticle.position.x = SIM_PHYS_X_SIZE;
      currentParticle.velocity.x = 0;
    }
    if (currentParticle.position.y < 0) {
      currentParticle.position.y = 0;
      currentParticle.velocity.y = 0;
    }
    if (currentParticle.position.y > SIM_PHYS_Y_SIZE) {
      currentParticle.position.y = SIM_PHYS_Y_SIZE;
      currentParticle.velocity.y = 0;
    }

    // check current cell it resides in, if its solid, push it out backwards
    // simply just undo the velocity movement done
    if (currentCell->state == SIM_SOLID) {
      Vec2_t reversedVelocity =
          ScalarMult_V2(currentParticle.velocity, (float)-0.75);
      currentParticle.position =
          AddVectors_V2(currentParticle.position, reversedVelocity);
    }
  }
}

void Sim_Particle_PushParticlesApart() {
  // count particles per cell
}

void Sim_Particle_Step() {
  Vec2_t GravImpact = {.x = 0, .y = SIM_GRAV * SIM_DELTATIME};

  // for each particle, just move particle based on its velocity
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    // change velocity by adding gravity
    particle_array[k].velocity =
        AddVectors_V2(particle_array[k].velocity, GravImpact);
    Vec2_t adjustedVelo = {.x = SIM_DELTATIME * particle_array[k].velocity.x,
                           .y = SIM_DELTATIME * particle_array[k].velocity.y};

    // update position by adding velocity to it
    particle_array[k].position =
        AddVectors_V2(particle_array[k].position, adjustedVelo);
  }

  // handle collisions
  Sim_Particle_HandleCellCollisions();
  for (int k = 0; k < SIM_OBSTACLE_COUNT; k++) {
    Sim_Particle_t currentObstacle = obstacle_array[k];
    Sim_Particle_HandleObstacleCollisions(currentObstacle);
  }
}

void Sim_Particle_Init() {
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    particle_array[k].state = SIM_WATER;
    particle_array[k].radius = SIM_PARTICLE_RADIUS;
    Vec2_t initial_pos = {.x = (float)(k % SIM_PHYS_X_SIZE),
                          .y = (float)(k % SIM_PHYS_Y_SIZE)};
    //particle_array[k].position = initial_pos;
  }
}

void Sim_Grid_Init() {
  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      if (k > 0) {
        grid_array[i][k].state = SIM_WATER;
      } else {
        grid_array[i][k].state = SIM_AIR;
      }
    }
  }
}

void Sim_PIC_Step() {}

void Sim_FLIP_Step() {}

void Sim_Grid_Step() {
  float divergence = 0;
  int netState = 0;
  float leftVelo, rightVelo, upVelo, downVelo;

  // essentially ensure the fluid is incompressible
  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      if (grid_array[i][k].state != SIM_WATER)
        continue;
      // update velocity of each cell
      // grid_array[i][k].velocity.y += SIM_DELTATIME * SIM_GRAV;

      // divergence step
      divergence = 0;
      netState = 0;

      if (i > 0 && grid_array[i - 1][k].state != SIM_SOLID) {
        leftVelo = grid_array[i - 1][k].x;
        netState++;
      } else {
        leftVelo = 0;
      }

      if (i < SIM_PHYS_X_SIZE - 1 && grid_array[i + 1][k].state != SIM_SOLID) {
        rightVelo = grid_array[i + 1][k].x;
        netState++;
      } else {
        rightVelo = 0;
      }

      if (k > 0 && grid_array[i][k - 1].state != SIM_SOLID) {
        downVelo = grid_array[i][k - 1].y;
        netState++;
      } else {
        downVelo = 0;
      }

      if (k < SIM_PHYS_Y_SIZE - 1 && grid_array[i][k + 1].state != SIM_SOLID) {
        upVelo = grid_array[i][k + 1].y;
        netState++;
      } else {
        upVelo = 0;
      }

      // if we end up with netState == 0, no movement possible!
      if (netState == 0)
        continue;

      divergence = upVelo - downVelo + rightVelo - leftVelo;

      // set values using divergence
      if (i > 0 && grid_array[i - 1][k].state != SIM_SOLID) {
        grid_array[i - 1][k].velocity.x -= divergence / netState;
      }
      if (i < SIM_PHYS_X_SIZE && grid_array[i + 1][k].state != SIM_SOLID) {
        grid_array[i + 1][k].velocity.x += divergence / netState;
      }
      if (k > 0 && grid_array[i][k - 1].state != SIM_SOLID) {
        grid_array[i][k - 1].velocity.y -= divergence / netState;
      }
      if (k < SIM_PHYS_X_SIZE && grid_array[i][k + 1].state != SIM_SOLID) {
        grid_array[i][k + 1].velocity.y += divergence / netState;
      }
    }
  }
}

void Sim_TransferVelocities(int toGrid) {
  if (toGrid) {
    // transferring from particles to grid
    for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
      Sim_Particle_t currentParticle = particle_array[k];
      Sim_Cell_t *currentCell = GetCellFromPosition(currentParticle.position);
      if (currentCell->state == SIM_AIR) {
        currentCell->state = SIM_WATER;
      }
    }
  } else {
    // transferrning from grid to particles
  }
}

void Sim_Physics_Step() {
  for (int k = 0; k < SIM_ITERATIONS; k++) {
    Sim_Particle_Step();       // handle particle movement + gravity
    Sim_TransferVelocities(1); // transfer particle -> grid velocities
    // update particle density?
    Sim_Grid_Step(); // solve incompressibility

    Sim_TransferVelocities(0); // transfer grid -> particle velocities
  }
}

Sim_Cell_t *GetCellFromPosition(Vec2_t position) {
  int x = (int)position.x;
  int y = (int)position.y;
  if (x < 0 || x >= SIM_PHYS_X_SIZE || y < 0 || y >= SIM_PHYS_Y_SIZE) {
    return NULL;
  }
  return &grid_array[x][y];
}

// FOR SERIAL MONITOR USE:
extern uint8_t tx_buff[sizeof(PREAMBLE) +
                       SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE + sizeof(SUFFIX) +
                       2];
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

  /**/
  for (int i = 0; i < SIM_X_SIZE * SIM_Y_SIZE; i++) {
    // if (i % 2 == 0) continue;
    // char pixel = i / 4;
    char pixel = sim_time * 32;
    tx_buff[tx_buff_len++] = pixel;
    // print_msg(&pixel);
  }

  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      if (k == SIM_PHYS_Y_SIZE - 1 && i == SIM_PHYS_X_SIZE - 1) {
        continue;
      }
      char pixel = 0x00;
      // pixel = (char) Magnitude_V2(grid_array[i][k].velocity);

      if (grid_array[i][k].state == SIM_WATER) {
        pixel = WATER_COLOR_R;
      } else {
        pixel = AIR_COLOR_R;
      }

      tx_buff[tx_buff_len++] = pixel;
    }
  }

  // Load the END suffix message to the end of the message.
  for (int i = 0; i < sizeof(SUFFIX); i++) {
    tx_buff[tx_buff_len++] = SUFFIX[i];
  }

  // print_msg((char*) tx_buff);

  while (HAL_UART_GetState(&huart3) != HAL_UART_STATE_READY) {
  }
  HAL_UART_Transmit_DMA(&huart3, (uint8_t *)tx_buff,
                        sizeof(PREAMBLE) + SIM_X_SIZE * SIM_Y_SIZE);
}

void print_msg(char *msg) {
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
}
