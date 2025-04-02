#include "fluid_sim.h"
#include "physics.h"
#include "oled.h"

// FLUID SIM Initializations
/*
Notes:
Heavily Based on the work done by Matthias Müller and his YouTube channel "Ten
Minute Physics"
*/

char msg[100];
/*
Much of the simulation ideas are heavily based on the TenMinutePhysics code
Key difference is this is adapted from JavaScript into C code, with minor
modifications

https://github.com/matthias-research/pages/blob/master/tenMinutePhysics/18-flip.html

*/

// utility functions
Sim_Cell_t *GetCellFromPosition(Vec2_t position) {
  int x = FLOAT_TO_INT(position.x);
  int y = FLOAT_TO_INT(position.y);
  if (x < 0 || x > SIM_PHYS_X_SIZE - 1 || y < 0 || y > SIM_PHYS_Y_SIZE - 1) {
    return NULL;
  }
  return &grid_array[x][y];
}

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
    particle_array[k].state = SIM_WATER;
    particle_array[k].radius = SIM_PARTICLE_RADIUS;
    particle_array[k].position = BlankVector_V2();
    particle_array[k].velocity = BlankVector_V2();

    initial_pos = (Vec2_t){.x = (float)(k % ((SIM_PHYS_X_SIZE - 1) / 2)) / 2 +
                                (SIM_PHYS_X_SIZE / 4),
                           .y = (float)(k % ((SIM_PHYS_Y_SIZE - 1) / 2)) / 2 +
                                (2 * SIM_PHYS_Y_SIZE) / 3};
    /*
    initial_pos =
        (Vec2_t){.x = (float)(k % (SIM_PHYS_X_SIZE / 2)),
                 .y = (float)(SIM_PHYS_Y_SIZE - (k / (SIM_PHYS_X_SIZE / 2)))};
    */
    particle_array[k].position = initial_pos;
  }
  Sim_Particle_PushParticlesApart();
  Vec2_t initial_gravity = {.x = 0, .y = -SIM_GRAV};
  GravityVector = initial_gravity;
  /*
  sprintf(msg, "Gravity: (%f, %f)\n", GravityVector.x, GravityVector.y);
  print_msg(msg);
  */
}

void Sim_Particle_Step() {
  Vec2_t GravImpact =
      ScalarMult_V2(GravityVector, SIM_DELTATIME / SIM_ITERATIONS);

  // for each particle, just move particle based on its velocity
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    // change velocity by adding gravity
    particle_array[k].velocity =
        AddVectors_V2(particle_array[k].velocity, GravImpact);
    Vec2_t adjustedVelo = {
        .x = SIM_DELTATIME * particle_array[k].velocity.x / SIM_ITERATIONS,
        .y = SIM_DELTATIME * particle_array[k].velocity.y / SIM_ITERATIONS};
    /*
    sprintf(msg, "%d: adjustedVelo: (%f, %f)\n", k, adjustedVelo.x,
            adjustedVelo.y);
    print_msg(msg);
    */
    // update position by adding velocity to it
    /*
    sprintf(msg, "PreMove %d: (%f, %f)\n", k, particle_array[k].position.x,
            particle_array[k].position.y);
    print_msg(msg);
    */
    particle_array[k].position =
        AddVectors_V2(particle_array[k].position, adjustedVelo);

    /*
    sprintf(msg, "Update %d: (%f, %f), Velocity = (%f, %f)\n", k,
            particle_array[k].position.x, particle_array[k].position.y,
            particle_array[k].velocity.x, particle_array[k].velocity.y);
    print_msg(msg);
    */
  }

  // handle collisions
  Sim_Particle_HandleCellCollisions();
  for (int k = 0; k < SIM_OBSTACLE_COUNT; k++) {
    Sim_Particle_t currentObstacle = obstacle_array[k];
    Sim_Particle_HandleObstacleCollisions(currentObstacle);
  }
}


void Sim_Particle_PushParticlesApart() {
  // reset particle_count for all cells
  // print_msg("reset particle counts\n");
  for (int separate_iter = 0; separate_iter < SIM_PARTICLE_SEPARATE_ITERATIONS;
       separate_iter++) {

    // reset grid particle count and linked list
    for (int k = 0; k < SIM_PHYS_X_SIZE; k++) {
      for (int j = 0; j < SIM_PHYS_Y_SIZE; j++) {
        grid_array[k][j].particle_count = 0;
        grid_array[k][j].head = NULL;
        grid_array[k][j].tail = NULL;
      }
    }

    // count particles in cells
    int count = 0;
    //print_msg("counting particles in cell\n");
    for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
      particle_array[k].next = NULL;

      Sim_Cell_t *locatedCell = GetCellFromPosition(particle_array[k].position);
      if (locatedCell != NULL) {
        locatedCell->particle_count++;
        AddParticleToCellList(locatedCell, &particle_array[k]);
        count++;
      }
    }

    // push particles apart
    // sprintf(msg, "found particles in grid = %d\n", count);
    // print_msg(msg);
    // print_msg("actually separate particles\n");

    float min_dist = SIM_PARTICLE_RADIUS * 2;
    float min_dist_squared = min_dist * min_dist;

    // iterate through each cell, if there is particles there, separate them

    int split_count = 0;
    for (int x = 0; x < SIM_PHYS_X_SIZE; x++) {
      for (int y = 0; y < SIM_PHYS_Y_SIZE; y++) {
        if (grid_array[x][y].particle_count > 1) {
          // more than one particle, separate ALL particles in cell
          Sim_Particle_t *focusParticle = grid_array[x][y].head;
          Sim_Particle_t *otherParticle = NULL;

          while (focusParticle->next != NULL && focusParticle != NULL) {
            if (focusParticle) {
              otherParticle = focusParticle->next;
            } else {
              focusParticle = NULL;
              continue;
            }
            if (otherParticle == NULL) {
              focusParticle = NULL;
              continue;
            }
            // print_msg("something found\n");
            //  if here, both particles actually exist!
            Vec2_t negativePos = ScalarMult_V2(focusParticle->position, -1);
            Vec2_t deltaPos =
                AddVectors_V2(otherParticle->position, negativePos);
            float dist_between = Magnitude_V2(deltaPos);
            float dist_between_squared = dist_between * dist_between;

            if (dist_between_squared > min_dist_squared ||
                dist_between_squared == 0.0) {
              // skip
            } else {
              // actually separate particles, using min distance
              float separateFactor =
                  0.5 * (min_dist - dist_between) / dist_between;
              deltaPos = ScalarMult_V2(deltaPos, separateFactor);
              Vec2_t negative_deltaPos = ScalarMult_V2(deltaPos, -1);
              otherParticle->position =
                  AddVectors_V2(otherParticle->position, deltaPos);
              focusParticle->position =
                  AddVectors_V2(focusParticle->position, negative_deltaPos);
            }
            focusParticle = focusParticle->next;
            split_count++;
          }
        }
      }
    }
    Sim_Particle_HandleCellCollisions();
  }
}

// grid functions
void Sim_Grid_Init() {
  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      grid_array[i][k].state = SIM_AIR;
      if (k == 0 || k == SIM_PHYS_Y_SIZE - 1) {
        // grid_array[i][k].state = SIM_SOLID;
      } else if (i == 0 || i == SIM_PHYS_X_SIZE - 1) {
        // grid_array[i][k].state = SIM_SOLID;
      }
      grid_array[i][k].x = i;
      grid_array[i][k].y = k;
    }
  }
}

void Sim_Grid_Step() {
  float divergence = 0;
  int netState = 0;
  float leftVelo, rightVelo, upVelo, downVelo, selfVert, selfHori;
  Vec2_t GravImpact =
      ScalarMult_V2(GravityVector, SIM_DELTATIME / SIM_ITERATIONS);

  // essentially ensure the fluid is incompressible
  for (int i = 0; i < SIM_PHYS_X_SIZE; i++) {
    for (int k = 0; k < SIM_PHYS_Y_SIZE; k++) {
      if (grid_array[i][k].state != SIM_WATER) {
        if (grid_array[i][k].state == SIM_AIR) {
          grid_array[i][k].velocity.x = 0;
          grid_array[i][k].velocity.y = 0;
        }
        continue;
      }
      // update velocity of each cell
      // grid_array[i][k].velocity =
      //    AddVectors_V2(grid_array[i][k].velocity, GravImpact);

      // divergence step
      divergence = 0;
      netState = 1;

      if (i > 0 && grid_array[i - 1][k].state == SIM_WATER) {
        leftVelo = grid_array[i - 1][k].velocity.x;
        netState++;
      } else {
        leftVelo = 0;
      }

      if (i < SIM_PHYS_X_SIZE - 1 && grid_array[i + 1][k].state == SIM_WATER) {
        rightVelo = grid_array[i + 1][k].velocity.x;
        netState++;
      } else {
        rightVelo = 0;
      }

      if (k > 0 && grid_array[i][k - 1].state == SIM_WATER) {
        downVelo = grid_array[i][k - 1].velocity.y;
        netState++;
      } else {
        downVelo = 0;
      }

      if (k < SIM_PHYS_Y_SIZE - 1 && grid_array[i][k + 1].state == SIM_WATER) {
        upVelo = grid_array[i][k + 1].velocity.y;
        netState++;
      } else {
        upVelo = 0;
      }

      // if we end up with netState == 0, no movement possible!
      if (netState == 0) {
        continue;
      }

      divergence =
          SIM_OVERRELAXATION * (upVelo - downVelo + rightVelo - leftVelo +
                                Magnitude_V2(grid_array[i][k].velocity));

      // set values using divergence
      // if more downward than upward, push upward, and vice versa
      // same idea for horizontal

      // grid_array[i][k].velocity.y += divergence / (float)netState;
      // grid_array[i][k].velocity.x += divergence / (float)netState;

      if (fabsf(upVelo) > fabsf(downVelo)) {
        grid_array[i][k].velocity.y += divergence / (float)netState;
      } else {
        grid_array[i][k].velocity.y += -1 * divergence / (float)netState;
      }
      if (fabsf(leftVelo) > fabsf(rightVelo)) {
        grid_array[i][k].velocity.x += divergence / (float)netState;
      } else {
        grid_array[i][k].velocity.x += -1 * divergence / (float)netState;
      }

      /*
        if (i > 0 && grid_array[i - 1][k].state == SIM_WATER) {
          grid_array[i - 1][k].velocity.x -= divergence / netState;
        }
        if (i < SIM_PHYS_X_SIZE && grid_array[i + 1][k].state == SIM_WATER) {
          grid_array[i + 1][k].velocity.x += divergence / netState;
        }
        if (k > 0 && grid_array[i][k - 1].state == SIM_WATER) {
          grid_array[i][k - 1].velocity.y -= divergence / netState;
        }
        if (k < SIM_PHYS_X_SIZE && grid_array[i][k + 1].state == SIM_WATER) {
          grid_array[i][k + 1].velocity.y += divergence / netState;
        }
           */
    }
  }
}




void Sim_Particle_HandleObstacleCollisions(Sim_Particle_t obstacle) {
  // simply check if in radius
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {

    float dx = particle_array[k].position.x - obstacle.position.x;
    float dy = particle_array[k].position.y - obstacle.position.y;
    float dxy_squared = dx * dx + dy * dy;

    float min_distance = obstacle.radius + particle_array[k].radius;
    float min_dist_squared = min_distance * min_distance;
    if (dxy_squared < min_dist_squared) {
      // collision, simply inherit velocity
      particle_array[k].velocity.x = obstacle.velocity.x;
      particle_array[k].velocity.y = obstacle.velocity.y;
    }
  }
}

void Sim_Particle_HandleCellCollisions() {
  // for each particle...
  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    // check boundary conditions
    Sim_Cell_t *currentCell = GetCellFromPosition(particle_array[k].position);
    if (particle_array[k].position.x < 0) {
      particle_array[k].position.x = 0;
      if (particle_array[k].velocity.x < 0) {
        particle_array[k].velocity.x = 0;
      }
    } else if (particle_array[k].position.x > SIM_PHYS_X_SIZE - 1) {
      particle_array[k].position.x = SIM_PHYS_X_SIZE - 1;
      if (particle_array[k].velocity.x > 0) {
        particle_array[k].velocity.x = 0;
      }
    }
    if (particle_array[k].position.y < 0) {
      // sprintf(msg, "%d: collision at bottom!\n", k);
      // print_msg(msg);
      particle_array[k].position.y = 0;
      if (particle_array[k].velocity.y < 0) {
        particle_array[k].velocity.y = 0;
      }
    } else if (particle_array[k].position.y > SIM_PHYS_Y_SIZE - 1) {
      particle_array[k].position.y = SIM_PHYS_Y_SIZE - 1;
      if (particle_array[k].velocity.y > 0) {
        particle_array[k].velocity.y = 0;
      }
    }

    // check current cell it resides in, if its solid, push it out backwards
    // simply just undo the velocity movement done
    if (currentCell != NULL && currentCell->state == SIM_SOLID) {
      Vec2_t reversedVelocity =
          ScalarMult_V2(particle_array[k].velocity, (float)-0.25);
      particle_array[k].position =
          AddVectors_V2(particle_array[k].position, reversedVelocity);
      // particle_array[k].velocity = reversedVelocity;
    }
  }
}


void Sim_TransferVelocities(int toGrid) {
  if (toGrid) {
    // transferring from particles to grid
    // reset grid states, and recalculate after particle movement
    for (int k = 0; k < SIM_PHYS_X_SIZE; k++) {
      for (int j = 0; j < SIM_PHYS_Y_SIZE; j++) {
        if (grid_array[k][j].state == SIM_WATER) {
          grid_array[k][j].state = SIM_AIR;
        }
        grid_array[k][j].velocity = BlankVector_V2();
        grid_array[k][j].particle_count = 0;
      }
    }
    for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
      Sim_Cell_t *currentCell = GetCellFromPosition(particle_array[k].position);
      if (currentCell == NULL) {
        continue;
      }

      currentCell->particle_count++;
      if (currentCell->state == SIM_AIR) {
        currentCell->state = SIM_WATER;
      }
      // distance from left and bottom of current cell
      float dx = particle_array[k].position.x - currentCell->x;
      float dy = particle_array[k].position.y - currentCell->y;

      // calculate weights for corners (more transfer to closer corners)
      // probably shouldnt have undefined corners, not sure when that would come
      // up, but was mentioned in the video...
      // not using a staggered grid, so no need to worry about that
      // transferring velocities into corner
      float w1 = (1 - dx) * (1 - dy); // bot left corner
      float w2 = dx * (1 - dy);       // bot right corner
      float w3 = dx * dy;             // top right corner
      float w4 = (1 - dx) * dy;       // top left corner

      float transfer_deno = (w1 + w2 + w3 + w4);

      // for now, ignoring weighted transfers, gonna transfer all directly into
      // current cell cells for us are centered (ie center of cell is source of
      // velocity, etc)
      currentCell->velocity =
          AddVectors_V2(currentCell->velocity, particle_array[k].velocity);
    }
  } else {
    // transfering from grid to particles
    for (int x = 0; x < SIM_PHYS_X_SIZE; x++) {
      for (int y = 0; y < SIM_PHYS_Y_SIZE; y++) {

        if (grid_array[x][y].particle_count >= 1 &&
            grid_array[x][y].state == SIM_WATER) {
          // iterate through grid cell linked list
          int v1, v2, v3, v4;
          float d1, d2, d3, d4, netX, netY;
          Vec2_t velo1, velo2, velo3, velo4, netVelo;
          if (x > 0 && grid_array[x - 1][y].state == SIM_WATER) {
            v1 = 1;
            velo1 = grid_array[x - 1][y].velocity;
            d1 = velo1.x;
          }
          if (x < SIM_PHYS_X_SIZE - 1 &&
              grid_array[x + 1][y].state == SIM_WATER) {
            v2 = 1;
            velo2 = grid_array[x + 1][y].velocity;
            d2 = velo2.x;
          }
          if (y > 0 && grid_array[x][y - 1].state == SIM_WATER) {
            v3 = 1;
            velo3 = grid_array[x][y - 1].velocity;
            d3 = velo3.y;
          }
          if (y < SIM_PHYS_Y_SIZE - 1 &&
              grid_array[x][y + 1].state == SIM_WATER) {
            v4 = 1;
            velo4 = grid_array[x][y + 1].velocity;
            d4 = velo4.y;
          }
          netX = d1 + d2;
          netY = d3 + d4;
          netVelo.x = netX;
          netVelo.y = netY;

          Sim_Particle_t *focusParticle = grid_array[x][y].head;
          float inverse = (1 / (float)grid_array[x][y].particle_count);

          while (focusParticle != NULL) {
            Vec2_t changeVelo =
                ScalarMult_V2(grid_array[x][y].velocity, inverse);
            Vec2_t originalVelo =
                ScalarMult_V2(focusParticle->velocity, inverse);

            // focusParticle->velocity = AddVectors_V2(originalVelo,
            // changeVelo);
            focusParticle = focusParticle->next;
          }
        }

        /*
        for (int k = 0; k < grid_array[x][y].particle_count; k++) {
          float inverse = ((float)1 / (float)grid_array[x][y].particle_count);
          Vec2_t changeVelo = ScalarMult_V2(grid_array[x][y].velocity, inverse);
          particle_array[k].velocity = changeVelo;
        }
          */
      }
    }
  }
}

void Sim_Physics_Step() {
  //print_msg("physics step\n");
  for (int k = 0; k < SIM_ITERATIONS; k++) {
    //print_msg("particle step\n");
    Sim_Particle_Step(); // handle particle movement + gravity

    // print_msg("pushed particles apart\n");
    Sim_Particle_PushParticlesApart(); // separate particles from each other

    // print_msg("particle -> grid velocity transfer\n");
    Sim_TransferVelocities(1); // transfer particle -> grid velocities

    // update particle density?
    //print_msg("grid solver\n");
    Sim_Grid_Step(); // solve incompressibility

    // print_msg("grid -> particle velocity transfer\n");
    Sim_TransferVelocities(0); // transfer grid -> particle velocities
  }
}

void Sim_Physics_Init() {
  Sim_Grid_Init();
  Sim_Particle_Init();
}

// FOR SERIAL MONITOR USE:
extern uint16_t image_buff[SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE];
extern uint8_t tx_buff[sizeof(PREAMBLE) +
                       SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE + sizeof(SUFFIX) +
                       2];
extern size_t tx_buff_len;
extern UART_HandleTypeDef huart3;

void renderImage() {
  for (int k = 0; k < SIM_RENDER_X_SIZE * SIM_RENDER_Y_SIZE; k++) {
    image_buff[k] = BLACK; // Background color
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

  // iterating by particles

  for (int k = 0; k < SIM_PARTICLE_COUNT; k++) {
    Sim_Cell_t *locatedCell = GetCellFromPosition(particle_array[k].position);
    if (locatedCell) {
      uint8_t pixel = WATER_COLOR_R;

    int screen_x = SIM_RENDER_TO_PHYS_RATIO * particle_array[k].position.x;
    int screen_y = SIM_RENDER_TO_PHYS_RATIO *
                   (SIM_PHYS_Y_SIZE - particle_array[k].position.y);
    if (screen_y < 0) {
      screen_y = 0;
    } else if (screen_y > SIM_RENDER_Y_SIZE - 1) {
      screen_y = SIM_RENDER_Y_SIZE - 1;
    }
    if (screen_x < 0) {
      screen_x = 0;
    } else if (screen_x > SIM_RENDER_X_SIZE - 1) {
      screen_x = SIM_RENDER_X_SIZE - 1;
    }
    image_buff[(screen_y * SIM_RENDER_X_SIZE) + (screen_x)] = pixel;
    // image_buff[(screen_y * SIM_RENDER_X_SIZE) + (screen_x + 1)] = pixel;
    // image_buff[((screen_y + 1) * SIM_RENDER_X_SIZE) + (screen_x)] = pixel;
    // image_buff[((screen_y + 1) * SIM_RENDER_X_SIZE) + (screen_x + 1)] =
    // pixel;

      // sprintf(msg, "%d: (%d, %d) vs (%f, %f)\n", k, x, y,
      //         particle_array[k].position.x, particle_array[k].position.y);
      // print_msg(msg);

    } else {
      sprintf(msg, "renderImage(), OOB: %d: (%f, %f)\n", k,
              particle_array[k].position.x, particle_array[k].position.y);
      print_msg(msg);
      image_buff[0] = SOLID_COLOR_B;
      image_buff[1] = SOLID_COLOR_B;
    }
  }

  // iterating by cell (not as functional?)
  /*
  for (int x = 0; x < SIM_PHYS_X_SIZE; x++) {
    for (int y = 0; y < SIM_PHYS_Y_SIZE; y++) {
      uint8_t pixel = AIR_COLOR_B;

      if (grid_array[x][y].state == SIM_SOLID) {
        pixel = SOLID_COLOR_B;
      } else if (grid_array[x][y].state == SIM_WATER) {
        pixel = WATER_COLOR_B;
      }
      int screen_x = 2 * x;
      int screen_y = 2 * (SIM_PHYS_Y_SIZE - y);
      if (screen_y < 0) {
        screen_y = 0;
      } else if (screen_y > SIM_RENDER_Y_SIZE - 1) {
        screen_y = SIM_RENDER_Y_SIZE - 2;
      }
      image_buff[(screen_y)*SIM_RENDER_X_SIZE + (screen_x)] = pixel;
      image_buff[(screen_y)*SIM_RENDER_X_SIZE + (screen_x + 1)] = pixel;
      image_buff[((screen_y + 1) * SIM_RENDER_X_SIZE) + (screen_x)] = pixel;
      image_buff[((screen_y + 1) * SIM_RENDER_X_SIZE) + (screen_x + 1)] = pixel;
    }
  }
  */
  // print_msg("finished renderImage() call\n");
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
  for (int i = 0; i < sizeof(PREAMBLE); i++)
  {
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
