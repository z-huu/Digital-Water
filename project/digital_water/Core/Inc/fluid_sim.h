#ifndef __FLUID_SIM_H
#define __FLUID_SIM_H

#include "main.h"
#include "physics.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fluid Sim Dimensions (PMOD OLEDrgb screen is 96x64 pixels)
#define SIM_RENDER_X_SIZE 96
#define SIM_RENDER_Y_SIZE 64

#define SIM_RENDER_TO_PHYS_RATIO 2

#define SIM_PHYS_X_SIZE SIM_RENDER_X_SIZE / SIM_RENDER_TO_PHYS_RATIO
#define SIM_PHYS_Y_SIZE SIM_RENDER_Y_SIZE / SIM_RENDER_TO_PHYS_RATIO

#define SIM_X_SIZE SIM_RENDER_X_SIZE
#define SIM_Y_SIZE SIM_RENDER_Y_SIZE

#define SIM_GRAV ((float)5)

#define SIM_AIR 0
#define SIM_SOLID 1
#define SIM_WATER 2
#define SIM_OBSTACLE 3

#define SIM_PHYSICS_FPS 10
#define SIM_RENDER_FPS 10
#define SIM_DELAY_MS ((uint32_t)1000 / SIM_PHYSICS_FPS)

#define SIM_ITERATIONS 1
#define SIM_PARTICLE_COUNT 2048

#define SIM_PARTICLE_RADIUS ((float)1.0)

#define SIM_OBSTACLE_COUNT 0
#define SIM_DELTATIME ((float)(1) / (float)(SIM_PHYSICS_FPS * SIM_ITERATIONS))
#define SIM_PARTICLE_SEPARATE_ITERATIONS 1

#define SIM_OVERRELAXATION ((float)1.9) // should be between 1 to 2
// float to int macro found from StackOverflow:
// https://stackoverflow.com/questions/24723180/c-convert-floating-point-to-int
#define FLOAT_TO_INT(x) ((x) >= 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))

typedef struct particle {
  Vec2_t position;
  Vec2_t velocity;
  int state;
  float radius;
  struct particle *next;
} Sim_Particle_t;

typedef struct particle
{
  Vec2_t position;
  Vec2_t velocity;
  int state;
  float radius;
  struct particle *next;

} Sim_Particle_t;

typedef struct
{
  int state;
  int x;
  int y;
  float density;
  int particle_count;
  Vec2_t velocity;
  Sim_Particle_t *head;
  Sim_Particle_t *tail;
} Sim_Cell_t;

extern Sim_Cell_t grid_array[SIM_PHYS_X_SIZE][SIM_PHYS_Y_SIZE];
extern Sim_Particle_t particle_array[SIM_PARTICLE_COUNT];
extern Sim_Particle_t obstacle_array[SIM_OBSTACLE_COUNT];
extern Vec2_t GravityVector;

extern int sim_time;

// utility functions
Sim_Cell_t *GetCellFromPosition(Vec2_t position);

void AddParticleToCellList(Sim_Cell_t *cell, Sim_Particle_t *particle);

// main simulation functions

// fluid sim particle functions
Sim_Particle_t BlankParticle();

void Sim_Particle_Init();

void Sim_Particle_Step();

void Sim_Particle_HandleObstacleCollisions(Sim_Particle_t obstacle);

void Sim_Particle_HandleCellCollisions();

void Sim_Particle_PushParticlesApart();

// fluid sim grid functions
void Sim_Grid_Init();

void Sim_Grid_Step();

// overall physics functions
void Sim_Physics_Init();

void Sim_Physics_Step();

// Stuff related to Rendering (with SPI) (not finished, need more details)
#define SOLID_COLOR_R 0x10
#define SOLID_COLOR_G 0x10
#define SOLID_COLOR_B 0x10

#define WATER_COLOR_R 0x90
#define WATER_COLOR_G 0x90
#define WATER_COLOR_B 0x90

#define AIR_COLOR_R 0xFF
#define AIR_COLOR_G 0xFF
#define AIR_COLOR_B 0xFF

// Stuff related to Serial Monitor
#define PREAMBLE "\r\n!START!\r\n"
#define DELTA_PREAMBLE "\r\n!DELTA!\r\n"
#define SUFFIX "!END!\r\n"

#define DebugPrints 1

void renderImage();

void dummyImage();

void testPrint(void);

void print_msg(char *msg);
#endif