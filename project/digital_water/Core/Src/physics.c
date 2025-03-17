#include "physics.h"
#include <math.h>

float Magnitude_V2(Vec2_t vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y);
}

Vec2_t Normalize_V2(Vec2_t vector) {
  Vec2_t newVector;
  float magnitude = Magnitude_V2(vector);
  if (magnitude != 0) {
    newVector.x = vector.x / magnitude;
    newVector.y = vector.y / magnitude;
  } else {
    newVector.x = 0;
    newVector.y = 0;
  }
  return newVector;
}

float Magnitude_V3(Vec3_t vector) {
  Vec3_t newVector;
  float magnitude = Magnitude_V3(vector);
  if (magnitude != 0) {
    newVector.x = vector.x / magnitude;
    newVector.y = vector.y / magnitude;
    newVector.z = vector.z / magnitude;
  } else {
    newVector.x = 0;
    newVector.y = 0;
    newVector.z = 0;
  }
  return newVector;
}

Vec3_t Normalize_V3(Vec3_t vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}
