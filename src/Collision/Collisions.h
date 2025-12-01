#pragma once
#include "Entities/Entity.h"
// #include <memory>
#include <vector>

#include "Events/EventSystem.h"
#include "Events/EventTypes.h"
#include "Memory/MemoryPool.h"

struct CollisionEvent : public Event {
    inline static MemoryPool* CollisionMemoryPool;
    Entity* stat = nullptr;
    Entity* dyn = nullptr;
    CollisionData data_stat;
    CollisionData data_dyn;
    void* operator new(size_t size) {
      int sl_id = CollisionEvent::CollisionMemoryPool->alloc();
      if (sl_id != -1)
        return CollisionEvent::CollisionMemoryPool->getPtr(sl_id);
      return nullptr;
    }
    
    void operator delete(void* ptr) {
      CollisionEvent::CollisionMemoryPool->freeSlot(CollisionEvent::CollisionMemoryPool->getSlot(ptr));
    }

    CollisionEvent(Entity* p_stat, Entity* p_dyn, CollisionData pdata_stat, CollisionData pdata_dyn) 
    : stat(p_stat), dyn(p_dyn), data_stat(pdata_stat), data_dyn(pdata_dyn) {
       type = EventType::EVENT_TYPE_COLLISION;
    }
};


class CollisionEventHandler : public EventHandler {
  public:
  CollisionEventHandler() {
    CollisionEvent::CollisionMemoryPool = new MemoryPool(sizeof(CollisionEvent), 1024);
  }
  void OnEvent(Event* E) override {
        if (E->type == EventType::EVENT_TYPE_COLLISION) { 
            CollisionEvent* collisionEvent = static_cast<CollisionEvent*>(E);
            collisionEvent->stat->OnCollision(collisionEvent->dyn, &collisionEvent->data_stat);
            collisionEvent->dyn->OnCollision(collisionEvent->stat, &collisionEvent->data_dyn);
          }
    }

};

class CollisionSystem {
 EventManager* eventManager = nullptr;
  public:
  bool CheckCollision(const Entity *a, const Entity *b) const;
  bool CheckCollision(const SDL_FRect &a, const SDL_FRect &b) const;
  void SetEventManager(EventManager* ev) {eventManager = ev;}
  // Resolves penetration and sets grounded when landing on static bodies.
  void ProcessCollisions(std::vector<Entity *> &entities);
};
