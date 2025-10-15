#include "Physics.h"
#include "Entities/Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (
      !entity->hasPhysics || 
      entity->isStatic    
    )

    return;

  entity->velocity = add(entity->velocity, mul(deltaTime, entity->force));

  entity->position = add(entity->position, mul(deltaTime, entity->velocity));
}

void PhysicsSystem::ApplyPhysicsMultithreaded(const std::vector<Entity*>& entities) {
  if (entities.empty()) {
    return;
  }

  // Clear the persistent job queue
  jobSystem.ClearJobs();
  
  // Add all physics jobs to the persistent queue
  for (auto &entity : entities) {
    float entityDeltaTime = entity->timeline->getDeltaTime();
    jobSystem.AddJob([this, entity, entityDeltaTime]() {
      ApplyPhysics(entity, entityDeltaTime);
    });
  }
  
  // Execute all jobs using the persistent queue
  jobSystem.ExecuteJobs();
}
