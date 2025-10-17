#include "Physics.h"
#include "Entities/Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (!entity->physicsEnabled)
    return;

  // Check if entity is kinematic (static)
  if (entity->collisionEnabled && 
      entity->getComponent<CollisionComponent>("collision").isKinematic)
    return;

  PhysicsComponent& physics = entity->getComponent<PhysicsComponent>("physics");
  physics.velocity = add(physics.velocity, mul(deltaTime, physics.force));
  entity->position = add(entity->position, mul(deltaTime, physics.velocity));
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
