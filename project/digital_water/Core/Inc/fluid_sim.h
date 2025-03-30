#ifndef __FLUID_SIM_H
#define __FLUID_SIM_H

#include "main.h"
#include "physics.h"

// Fluid Sim Dimensions (PMOD OLEDrgb screen is 96x64 pixels)
#define SIM_RENDER_X_SIZE 96
#define SIM_RENDER_Y_SIZE 64

#define SIM_PHYS_X_SIZE (96 / 4)
#define SIM_PHYS_Y_SIZE (64 / 4)

#define SIM_X_SIZE SIM_RENDER_X_SIZE
#define SIM_Y_SIZE SIM_RENDER_Y_SIZE

#define SIM_GRAV -1

#define SIM_WATER 0
#define SIM_SOLID 1
#define SIM_AIR 2

#define SIM_PHYSICS_FPS 30
#define SIM_RENDER_FPS 30
#define SIM_DELAY_MS (uint32_t)1000 / SIM_PHYSICS_FPS

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

void Sim_Grid_Update();

// Stuff related to Rendering (with SPI)
#define SOLID_COLOR_R 0x50
#define SOLID_COLOR_G 0x50
#define SOLID_COLOR_B 0x50

#define WATER_COLOR_R 0x90
#define WATER_COLOR_G 0x90
#define WATER_COLOR_B 0x90

#define AIR_COLOR_R 0xF0
#define AIR_COLOR_G 0xF0
#define AIR_COLOR_B 0xF0

// Stuff related to Serial Monitor
#define PREAMBLE "\r\n!START!\r\n"
#define DELTA_PREAMBLE "\r\n!DELTA!\r\n"
#define SUFFIX "!END!\r\n"

void testPrint(void);

void print_msg(char *msg);
#endif