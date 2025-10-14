#pragma once
#include "Entity.h"
#include "JobSystem.h"
#include <vector>

class PhysicsSystem {
 public:
  PhysicsSystem(int numThreads) : jobSystem(numThreads) {}

  void ApplyPhysics(Entity *entity, float deltaTime);
  void ApplyPhysicsMultithreaded(const std::vector<Entity*>& entities);

 private:
  JobSystem jobSystem;
};
