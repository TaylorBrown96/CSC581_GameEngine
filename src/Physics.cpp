#include "Physics.h"

#include "Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (
      !entity->hasPhysics || 
      entity->isStatic    
    )

    return;

  entity->velocity = add(entity->velocity, mul(deltaTime, entity->force));

  entity->position = add(entity->position, mul(deltaTime, entity->velocity));
}
