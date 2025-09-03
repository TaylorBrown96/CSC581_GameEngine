#pragma once
#include "Entity.h"
#include <memory>
#include <vector>

class CollisionSystem {
public:
  bool CheckCollision(const Entity* a, const Entity* b) const;
  bool CheckCollision(const SDL_FRect& a, const SDL_FRect& b) const;

  // Resolves penetration and sets grounded when landing on static bodies.
  void ProcessCollisions(std::vector<std::shared_ptr<Entity>>& entities);
};
