#pragma once
#include "Entity.h"

class PhysicsSystem {
 public:
  PhysicsSystem() = default;

  void ApplyPhysics(Entity *entity, float deltaTime);
};
