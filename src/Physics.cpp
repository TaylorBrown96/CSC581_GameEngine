#include "Physics.h"
#include "Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (!entity->hasPhysics || entity->isStatic)
    return;
  entity->prevPosition = entity->position;

  // entity->prevX = entity->x;
  // entity->prevY = entity->y;

  entity->velocity = add(entity->velocity, mul(deltaTime, entity->force));

  // entity->velocityX += entity->forceX * deltaTime;
  // entity->velocityY += entity->forceY * deltaTime;

  entity->position = add(entity->position, mul(deltaTime, entity->velocity));
  // entity->x += entity->velocityX * deltaTime;
  // entity->y += entity->velocityY * deltaTime;
}
