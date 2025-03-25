#ifndef __PHYSICS_H
#define __PHYSICS_H

#include "main.h"

typedef struct v2{
  float x;
  float y;
} Vec2_t;


float Magnitude_V2(Vec2_t vector);
Vec2_t BlankVector_V2();
Vec2_t Normalize_V2(Vec2_t vector);
Vec2_t AddVectors_V2(Vec2_t A, Vec2_t B);
Vec2_t ScalarMult_V2(Vec2_t vector, float scalar);

typedef struct v3{
  float x;
  float y;
  float z;
} Vec3_t;

float Magnitude_V3(Vec3_t vector);
Vec3_t Normalize_V3(Vec3_t vector);

#endif