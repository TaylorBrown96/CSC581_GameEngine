#include <vec2.h>

vec2 add(vec2 a, vec2 b) { return {.x = a.x + b.x, .y = a.y + b.y}; }

vec2 neg(vec2 a) { return {.x = -a.x, .y = -a.y}; }

vec2 sub(vec2 a, vec2 b) { return add(a, neg(b)); }

float dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }

vec2 normalize(vec2 a) {
  float len = sqrt(a.x * a.x + a.y * a.y);
  return {.x = a.x / len, .y = a.y / len};
}

vec2 mul(float f, vec2 a) { return {.x = a.x * f, .y = a.y * f}; }

vec2 mulv(vec2 a, vec2 b) { return {.x = a.x * b.x, .y = a.y * b.y}; }
