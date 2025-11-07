#include "Collisions.h"

#include <SDL3/SDL.h>
#include "Math/vec2.h"
// #include <algorithm>

bool CollisionSystem::CheckCollision(const Entity *a, const Entity *b) const {
  SDL_FRect A = a->GetBounds();
  SDL_FRect B = b->GetBounds();
  return CheckCollision(A, B);
}

bool CollisionSystem::CheckCollision(const SDL_FRect &a,
                                     const SDL_FRect &b) const {
  return SDL_HasRectIntersectionFloat(&a, &b);
}

void CollisionSystem::ProcessCollisions(std::vector<Entity *> &entities) {

  const size_t n = entities.size();

  if (n == 0)
    return;
    
  for (size_t i = 0; i < n - 1; ++i) {
    for (size_t j = i + 1; j < n; ++j) {
      Entity *A = entities[i];
      Entity *B = entities[j];

      if (!A->collisionEnabled || !B->collisionEnabled)
        continue;

      SDL_FRect Ab = A->GetBounds();
      SDL_FRect Bb = B->GetBounds();
      if (!SDL_HasRectIntersectionFloat(&Ab, &Bb)) continue;

      // Decide dynamic vs static priority
      bool A_isKinematic = A->getComponent<CollisionComponent>("collision").isKinematic;
      bool B_isKinematic = B->getComponent<CollisionComponent>("collision").isKinematic;
      
      Entity *dyn = (A_isKinematic && !B_isKinematic) ? B : A;
      Entity *stat = (A_isKinematic && !B_isKinematic) ? A : B;

      // If both are dynamic or both static, just treat A as dyn, B as stat
      if (A_isKinematic == B_isKinematic) {
        dyn = A;
        stat = B;
      }

      SDL_FRect Db = dyn->GetBounds();
      SDL_FRect Sb = stat->GetBounds();

      SDL_FRect inter{};
      SDL_GetRectIntersectionFloat(&Db, &Sb, &inter);

      vec2 normals[4] = {
          {1.0, 0.0},   // RIGHT
          {-1.0, 0.0},  // LEFT
          {0.0, 1.0},   // BOTTOM
          {0.0, -1.0}   // TOP
      };

      vec2 db_collision_normal, sb_collision_normal;

      float minimum_penetration = std::min(inter.w, inter.h);

      if (inter.w < inter.h) /** side collision */ {
        if (Db.x < Sb.x) {
          db_collision_normal = normals[1];
        } else {
          db_collision_normal = normals[0];
        }
      } else /** top collision */ {
        if (Db.y < Sb.y) {
          db_collision_normal = normals[3];
        } else {
          db_collision_normal = normals[2];
        }
      }

      sb_collision_normal = neg(db_collision_normal);

      CollisionComponent& dynCollisionComponent = dyn->getComponent<CollisionComponent>("collision");
      CollisionComponent& statCollisionComponent = stat->getComponent<CollisionComponent>("collision");

      // If both entities are not ghost entities, resolve the collision
      if(!dynCollisionComponent.ghostEntity && !statCollisionComponent.ghostEntity) {
        if (!dynCollisionComponent.isKinematic && !statCollisionComponent.isKinematic) {
          stat->position = add(stat->position, mul(minimum_penetration * 0.5f,
                                                   sb_collision_normal));
          dyn->position = add(dyn->position, mul(minimum_penetration * 0.5f,
                                                 db_collision_normal));
        } else {
          dyn->position =
              add(dyn->position, mul(minimum_penetration, db_collision_normal));
        }
      }
      

      vec2 collision_point = {.x = inter.x + 0.5f * inter.w,
                              .y = inter.y + 0.5f * inter.h};

      CollisionData cd_dyn = {.point = collision_point,
                              .normal = db_collision_normal};

      CollisionData cd_stat = {.point = collision_point,
                               .normal = sb_collision_normal};


      // TODO
      eventManager->Raise(new CollisionEvent(dyn, stat, cd_dyn, cd_stat));
      // dyn->OnCollision(stat, &cd_dyn);
      // stat->OnCollision(dyn, &cd_stat);
    }
  }
}