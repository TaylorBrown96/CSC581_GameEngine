#pragma once
#include <math.h>

typedef struct vec2 {
  float x;
  float y;
} vec2;

vec2 add(vec2 a, vec2 b);

vec2 neg(vec2 a);

vec2 sub(vec2 a, vec2 b);

float dot(vec2 a, vec2 b);

vec2 normalize(vec2 a);

vec2 mul(float f, vec2 a);

vec2 mulv(vec2 a, vec2 b);
