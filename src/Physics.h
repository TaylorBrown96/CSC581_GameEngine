#pragma once
#include "Entity.h"

class PhysicsSystem {
private:
  float gravity;

public:
  PhysicsSystem() : gravity(980.0f) {} // Default gravity (pixels/second^2)

  void SetGravity(float value) { gravity = value; }
  float GetGravity() const { return gravity; }

  void ApplyPhysics(Entity *entity, float deltaTime);
  void ApplyGravity(Entity *entity, float deltaTime);
};
