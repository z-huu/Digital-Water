#include "fluid_sim.h"
#include "physics.h"


// FLUID SIM Initializations
/*
Notes:
Heavily Based on the work done by Matthias Müller and his YouTube channel "Ten
Minute Physics"
*/
extern Sim_Cell_t grid_array[SIM_PHYS_X_SIZE][SIM_PHYS_Y_SIZE];
extern Sim_Particle_t particle_array[SIM_PARTICLE_COUNT];
extern Sim_Particle_t obstacle_array[SIM_OBSTACLE_COUNT];
extern Vec2_t GravityVector;

extern int sim_time;
char msg[100];
/*
Much of the simulation ideas are heavily based on the TenMinutePhysics code
Key difference is this is adapted from JavaScript into C code, with minor
modifications

https://github.com/matthias-research/pages/blob/master/tenMinutePhysics/18-flip.html

*/

// utility functions
Sim_Cell_t *GetCellFromPosition(Vec2_t position) {
  int x = (int)position.x;
  int y = (int)position.y;
  if (x < 0 || x >= SIM_PHYS_X_SIZE || y < 0 || y >= SIM_PHYS_Y_SIZE) {
    return NULL;
  }
  return &grid_array[x][y];
}

/*
void AddParticleToCellList(Sim_Cell_t *cell, Sim_Particle_t *particle) {
  if (cell->head == NULL) {
    // no nodes, simply add to head!
    cell->head = particle;
    cell->tail = particle;
  } else {
    // append to tail
    Sim_Particle_t *currentParticle = cell->head;
    while (currentParticle->next != NULL) {
      currentParticle = currentParticle->next;
    }

    currentParticle->next = particle;
    cell->tail = particle;
  }
}
*/

Sim_Particle_t BlankParticle() {
  Sim_Particle_t blank;
  blank.state = SIM_AIR;
  blank.radius = SIM_PARTICLE_RADIUS;
  blank.position = BlankVector_V2();
  blank.velocity = BlankVector_V2();
  return blank;
}

// particle functions
void Sim_Particle_Init() {
  // positions should be between x = 0 to 46 and y = 0 to 32
  // want left side start with water ->
  Vec2_t initial_pos;
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    particle_array[k] = BlankParticle();
    Sim_Particle_t currentParticle = particle_array[k];
    currentParticle.state = SIM_WATER;
    currentParticle.radius = SIM_PARTICLE_RADIUS;
    currentParticle.position = BlankVector_V2();
    currentParticle.velocity = BlankVector_V2();

    initial_pos = (Vec2_t){.x = ((float)(SIM_PHYS_X_SIZE) / (float)2),
                           .y = (float)(SIM_PHYS_Y_SIZE / 2)};

    // currentParticle.position = initial_pos;
    // currentParticle.position.x = initial_pos.x;
    // currentParticle.position.y = initial_pos.y;
    currentParticle.position =
        AddVectors_V2(currentParticle.position, initial_pos);
    sprintf(msg, "Init %d: (%f, %f)\n", k, currentParticle.position.x,
            currentParticle.position.y);
    print_msg(msg);
  }
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    sprintf(msg, "Re Init %d: (%f, %f)\n", k, particle_array[k].position.x,
            particle_array[k].position.y);
    print_msg(msg);
  }

  Vec2_t initial_gravity = {.x = 0, .y = -SIM_GRAV};
  GravityVector = initial_gravity;
  sprintf(msg, "Gravity: (%f, %f)\n", GravityVector.x, GravityVector.y);
  print_msg(msg);
}

void Sim_Particle_Step() {
  Vec2_t GravImpact = ScalarMult_V2(GravityVector, SIM_DELTATIME);

  // for each particle, just move particle based on its velocity
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    // change velocity by adding gravity
    Sim_Particle_t currentParticle = particle_array[k];

    currentParticle.velocity =
        AddVectors_V2(currentParticle.velocity, GravImpact);
    Vec2_t adjustedVelo = {.x = SIM_DELTATIME * currentParticle.velocity.x,
                           .y = SIM_DELTATIME * currentParticle.velocity.y};
    sprintf(msg, "%d: adjustedVelo: (%f, %f)\n", k, adjustedVelo.x,
            adjustedVelo.y);
    print_msg(msg);
    // update position by adding velocity to it
    sprintf(msg, "PreMove %d: (%f, %f)\n", k, currentParticle.position.x,
            currentParticle.position.y);
    print_msg(msg);
    currentParticle.position =
        AddVectors_V2(currentParticle.position, adjustedVelo);
    // currentParticle.position.x += adjustedVelo.x;
    // currentParticle.position.y += adjustedVelo.y;
    sprintf(msg, "Update %d: (%f, %f), Velocity = (%f, %f)\n", k,
            currentParticle.position.x, currentParticle.position.y,
            currentParticle.velocity.x, currentParticle.velocity.y);
    print_msg(msg);
  }

  // handle collisions
  // Sim_Particle_HandleCellCollisions();
  for (int k = 0; k < SIM_OBSTACLE_COUNT; k++) {
    Sim_Particle_t currentObstacle = obstacle_array[k];
    // Sim_Particle_HandleObstacleCollisions(currentObstacle);
  }
}

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

/*
void Sim_Particle_PushParticlesApart() {
  // reset particle_count for all cells
  print_msg("reset particle counts\n");

  for (int k = 0; k < SIM_PHYS_X_SIZE; k++) {
    for (int j = 0; j < SIM_PHYS_Y_SIZE; j++) {
      Sim_Cell_t locatedCell = grid_array[k][j];
      locatedCell.particle_count = 0;
      locatedCell.head = NULL;
    }
  }

  // count particles in cells
  print_msg("counting particles in cell\n");
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    Sim_Particle_t currentParticle = particle_array[k];
    currentParticle.next = NULL;

    Sim_Cell_t *locatedCell = GetCellFromPosition(currentParticle.position);
    if (locatedCell) {
      locatedCell->particle_count++;
      AddParticleToCellList(locatedCell, &currentParticle);
    }
  }

  // push particles apart
  print_msg("actually separate particles\n");

  float min_dist = SIM_PARTICLE_RADIUS * 2;
  float min_dist_squared = min_dist * min_dist;

  for (int iter = 0; iter < SIM_PARTICLE_SEPARATE_ITERATIONS; iter++) {
    for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
      Sim_Particle_t currentParticle = particle_array[k];

      Sim_Cell_t *initial_cell = GetCellFromPosition(currentParticle.position);

      int left_end = 0;
      int right_end = SIM_PHYS_X_SIZE - 1;
      if (initial_cell->x - 1 > 0) {
        left_end = initial_cell->x - 1;
      }
      if (initial_cell->x + 1 < SIM_PHYS_X_SIZE - 1) {
        right_end = initial_cell->x + 1;
      }

      int bottom_end = 0;
      int top_end = SIM_PHYS_Y_SIZE - 1;
      if (initial_cell->y - 1 > 0) {
        bottom_end = initial_cell->y - 1;
      }
      if (initial_cell->y + 1 < SIM_PHYS_Y_SIZE - 1) {
        top_end = initial_cell->y + 1;
      }

      // iterate all adjacent cells
      for (int x_ind = left_end; x_ind <= right_end; x_ind++) {
        for (int y_ind = bottom_end; y_ind <= top_end; y_ind++) {
          Vec2_t cellPos = {.x = (float)x_ind, .y = (float)y_ind};
          Sim_Cell_t *currentCell = GetCellFromPosition(cellPos);
          Sim_Particle_t *focusParticle = currentCell->head;
          // iterate through all particles within this cell
          while (focusParticle->next != NULL) {
            if (focusParticle == &currentParticle) {
              // ignore self
              focusParticle = focusParticle->next;
            } else {
              Vec2_t negativePos = ScalarMult_V2(focusParticle->position, -1);
              Vec2_t deltaPos =
                  AddVectors_V2(currentParticle.position, negativePos);
              float dist_between = Magnitude_V2(deltaPos);
              float dist_between_squared = dist_between * dist_between;

              if (dist_between_squared > min_dist_squared ||
                  dist_between_squared == 0.0) {
                // skip
                focusParticle = focusParticle->next;
              } else {
                // actually separate particles, using min distance
                float separateFactor =
                    0.5 * (min_dist - dist_between) / dist_between;
                deltaPos = ScalarMult_V2(deltaPos, separateFactor);
                Vec2_t negative_deltaPos = ScalarMult_V2(deltaPos, -1);
                currentParticle.position =
                    AddVectors_V2(currentParticle.position, deltaPos);
                focusParticle->position =
                    AddVectors_V2(focusParticle->position, negative_deltaPos);
              }
            }
          }
        }
      }
    }
  }
}
*/

// grid functions
void Sim_Grid_Init() {
  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      grid_array[i][k].x = i;
      grid_array[i][k].y = k;
    }
  }
}

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

// *** To be completed!
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
    print_msg("particle step\n");
    Sim_Particle_Step(); // handle particle movement + gravity

    // print_msg("pushed particles apart\n");
    // Sim_Particle_PushParticlesApart(); // separate particles from each other

    // print_msg("particle -> grid velocity transfer\n");
    // Sim_TransferVelocities(1); // transfer particle -> grid velocities

    // update particle density?
    // print_msg("grid solver\n");
    // Sim_Grid_Step(); // solve incompressibility

    // print_msg("grid -> particle velocity transfer\n");
    // Sim_TransferVelocities(0); // transfer grid -> particle velocities
  }
}

void Sim_Physics_Init() {
  Sim_Particle_Init();
  Sim_Grid_Init();
}

// FOR SERIAL MONITOR USE:
extern uint8_t image_buff[SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE];
extern uint8_t tx_buff[sizeof(PREAMBLE) +
                       SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE + sizeof(SUFFIX) +
                       2];
extern size_t tx_buff_len;
extern UART_HandleTypeDef huart3;

void renderImage() {
  for (int k = 0; k < SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE; k++) {
    image_buff[k] = AIR_COLOR_B;
  }
  /*
for (int k = 0; k < SIM_PHYS_X_SIZE; k++) {
  for (int j = 0; j < SIM_PHYS_Y_SIZE; j++) {
    uint8_t pixel = 0;
    if (grid_array[j][k].state == SIM_WATER) {
      pixel = WATER_COLOR_R;
    } else {
      pixel = AIR_COLOR_R;
    }

    image_buff[2 * k][2 * j] = pixel;
    image_buff[(2 * k) + 1][2 * j] = pixel;
    image_buff[2 * k][(2 * j) + 1] = pixel;
    image_buff[(2 * k) + 1][(2 * k) + 1] = pixel;
  }
}
  */
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    Sim_Particle_t currentParticle = particle_array[k];
    Sim_Cell_t *locatedCell = GetCellFromPosition(currentParticle.position);
    int x = locatedCell->x;
    int y = locatedCell->y;
    uint8_t pixel = WATER_COLOR_R;

    image_buff[(2 * x * SIM_RENDER_X_SIZE) + (2 * y)] = pixel;
    image_buff[(2 * x * SIM_RENDER_X_SIZE) + (2 * y + 1)] = pixel;
    image_buff[((2 * x + 1) * SIM_RENDER_X_SIZE) + (2 * y)] = pixel;
    image_buff[((2 * x + 1) * SIM_RENDER_X_SIZE) + (2 * y + 1)] = pixel;
    sprintf(msg, "%d: (%d, %d) vs (%f, %f)\n", k, x, y,
            currentParticle.position.x, currentParticle.position.y);
    print_msg(msg);
  }
  print_msg("finished renderImage() call\n");
}

void dummyImage() {

  for (int k = 0; k < SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE; k++) {
    image_buff[k] = AIR_COLOR_R;
  }

  uint8_t pixel = WATER_COLOR_R;

  for (int k = 0; k < SIM_PHYS_X_SIZE; k++) {
    for (int j = 0; j < SIM_PHYS_Y_SIZE; j++) {
      if (k % 2 || j % 2) {
        pixel = WATER_COLOR_R;
      } else {
        pixel = SOLID_COLOR_R;
      }
      image_buff[(2 * k * SIM_RENDER_X_SIZE) + (2 * j)] = pixel;
      image_buff[(2 * k * SIM_RENDER_X_SIZE) + (2 * j + 1)] = pixel;
      image_buff[((2 * k + 1) * SIM_RENDER_X_SIZE) + (2 * j)] = pixel;
      image_buff[((2 * k + 1) * SIM_RENDER_X_SIZE) + (2 * j + 1)] = pixel;
    }
  }

  /*
  image_buff[(0 * SIM_RENDER_X_SIZE) + 0] = pixel;
  image_buff[(0 * SIM_RENDER_X_SIZE) + 1] = pixel;
  image_buff[(1 * SIM_RENDER_X_SIZE) + 0] = pixel;
  image_buff[(1 * SIM_RENDER_X_SIZE) + 1] = pixel;
    */
}

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

  /*
  for (int i = 0; i < SIM_X_SIZE * SIM_Y_SIZE; i++) {
    // if (i % 2 == 0) continue;
    // char pixel = i / 4;
    char pixel = sim_time * 32;
    tx_buff[tx_buff_len++] = pixel;
    // print_msg(&pixel);
  }
    */

  for (int k = 0; k < SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE; k++) {
    tx_buff[tx_buff_len++] = image_buff[k];
  }
  /*
for (int i = 0; i < SIM_RENDER_X_SIZE; i++) {
for (int k = 0; k < SIM_RENDER_Y_SIZE; k++) {
  /*
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
  *
  tx_buff[tx_buff_len++] = image_buff[i * SIM_RENDER_X_SIZE + k];
}
}
*/
  // print_msg("finished loading image_buff onto tx_buff\n");
  //  Load the END suffix message to the end of the message.
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
  if (DebugPrints) {
    HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
  }
}
