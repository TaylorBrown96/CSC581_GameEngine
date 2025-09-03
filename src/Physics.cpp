#pragma once
#include "Physics.h"
#include "Entity.h"

void PhysicsSystem::ApplyPhysics(Entity* entity, float deltaTime) {
    if (!entity->hasPhysics || entity->isStatic) return;

    entity->prevX = entity->x;
    entity->prevY = entity->y;

    if (entity->affectedByGravity) {
        ApplyGravity(entity, deltaTime);
    }
    entity->x += entity->velocityX * deltaTime;
    entity->y += entity->velocityY * deltaTime;
}

void PhysicsSystem::ApplyGravity(Entity* entity, float deltaTime) {
    entity->velocityY += gravity * deltaTime;
}