#include "Physics.h"
#include "Entity.h"

void PhysicsSystem::ApplyPhysics(Entity *entity, float deltaTime) {
  if (!entity->hasPhysics || entity->isStatic)
    return;

  entity->prevX = entity->x;
  entity->prevY = entity->y;

  entity->velocityX += entity->forceX * deltaTime;
  entity->velocityY += entity->forceY * deltaTime;

  entity->x += entity->velocityX * deltaTime;
  entity->y += entity->velocityY * deltaTime;
}
