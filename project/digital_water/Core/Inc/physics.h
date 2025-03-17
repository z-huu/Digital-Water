#ifndef __PHYSICS_H
#define __PHYSICS_H

#include "main.h"

typedef struct {
  float x;
  float y;
} Vec2_t;

#define ZERO_VEC2 (Vec2_t )
float Magnitude_V2(Vec2_t vector);
Vec2_t Normalize_V2(Vec2_t vector);

typedef struct {
  float x;
  float y;
  float z;
} Vec3_t;

float Magnitude_V3(Vec3_t vector);
Vec3_t Normalize_V3(Vec3_t vector);

#endif