#include <algorithm>

#include "Entity.h"

void EntityManager::AddEntity(Entity *entity) { entities.push_back(entity); }

void EntityManager::RemoveEntity(Entity *entity) {

  entities.erase(std::remove(entities.begin(), entities.end(), entity),
                 entities.end());
  // delete entity;
}

void EntityManager::ClearAllEntities() {
  entities.clear(); 
}

