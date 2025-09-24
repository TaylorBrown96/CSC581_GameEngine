#include "Physics.h"
#include "Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (
      !entity->hasPhysics || 
      entity->isStatic || 
      entity->timeline->getState() == Timeline::State::PAUSE
    )

    return;

  deltaTime = entity->timeline->getAbsoluteScale() * deltaTime; // Apply timeline scale to deltaTime

  entity->velocity = add(entity->velocity, mul(deltaTime, entity->force));

  entity->position = add(entity->position, mul(deltaTime, entity->velocity));
}
