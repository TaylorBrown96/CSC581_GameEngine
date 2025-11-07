#pragma once
#include "Entities/Entity.h"
// #include <memory>
#include <vector>

#include "Events/EventSystem.h"
#include "Events/EventTypes.h"

struct CollisionEvent : public Event {
    Entity* stat = nullptr;
    Entity* dyn = nullptr;
    CollisionData data_stat;
    CollisionData data_dyn;
    CollisionEvent(Entity* p_stat, Entity* p_dyn, CollisionData pdata_stat, CollisionData pdata_dyn) 
    : stat(p_stat), dyn(p_dyn), data_stat(pdata_stat), data_dyn(pdata_dyn) {
       
    }
};


class CollisionEventHandler : public EventHandler {
  void OnEvent(Event* E) override {
        if (E->type == EventType::EVENT_TYPE_COLLISION) { 
            CollisionEvent* collisionEvent = static_cast<CollisionEvent*>(E);
            collisionEvent->stat->OnCollision(collisionEvent->dyn, &collisionEvent->data_stat);
            collisionEvent->dyn->OnCollision(collisionEvent->stat, &collisionEvent->data_dyn);
          }
    }

};

class CollisionSystem {
 public:
  bool CheckCollision(const Entity *a, const Entity *b) const;
  bool CheckCollision(const SDL_FRect &a, const SDL_FRect &b) const;

  // Resolves penetration and sets grounded when landing on static bodies.
  void ProcessCollisions(std::vector<Entity *> &entities);
};
