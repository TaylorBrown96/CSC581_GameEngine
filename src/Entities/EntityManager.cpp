#include <algorithm>

#include "Entity.h"

void EntityManager::AddEntity(Entity *entity) { entities.push_back(entity); }

void EntityManager::RemoveEntity(Entity *entity) {
  delete entity;
  entities.erase(std::remove(entities.begin(), entities.end(), entity),
                 entities.end());
}

void EntityManager::ClearAllEntities() {
  for (int i = 0; i < entities.size(); i++) {
    delete entities[i];
  }
  entities.clear(); 
}

