#include "Collisions.h"
#include <SDL3/SDL.h>
#include <algorithm>

bool CollisionSystem::CheckCollision(const Entity* a, const Entity* b) const {
  SDL_FRect A = a->GetBounds();
  SDL_FRect B = b->GetBounds();
  return CheckCollision(A, B);
}

bool CollisionSystem::CheckCollision(const SDL_FRect& a, const SDL_FRect& b) const {
  return SDL_HasRectIntersectionFloat(&a, &b);
}

void CollisionSystem::ProcessCollisions(std::vector<Entity*>& entities) {
    for (auto& e : entities) if (!e->isStatic) e->grounded = false;

    const size_t n = entities.size();
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            Entity* A = entities[i];
            Entity* B = entities[j];

            SDL_FRect Ab = A->GetBounds();
            SDL_FRect Bb = B->GetBounds();
            if (!SDL_HasRectIntersectionFloat(&Ab, &Bb)) continue;

            // Decide dynamic vs static priority
            Entity* dyn  = (A->isStatic && !B->isStatic) ? B : A;
            Entity* stat = (A->isStatic && !B->isStatic) ? A : B;

            // If both are dynamic or both static, just treat A as dyn, B as stat
            if (A->isStatic == B->isStatic) { dyn = A; stat = B; }

            SDL_FRect Db = dyn->GetBounds();
            SDL_FRect Dprev = dyn->GetPrevBounds();
            SDL_FRect Sb = stat->GetBounds();

            SDL_FRect inter{};
            SDL_GetRectIntersectionFloat(&Db, &Sb, &inter);

            // One-way: only resolve if we are moving downward and crossed the top
            if (stat->isOneWay) {
                float prevBottom = Dprev.y + Dprev.h;
                float currBottom = Db.y + Db.h;
                bool movingDown = dyn->velocityY >= 0.0f;
                bool crossedTop = (prevBottom <= Sb.y) && (currBottom >= Sb.y);

                if (inter.h <= inter.w && movingDown && crossedTop) {
                    dyn->y = Sb.y - dyn->height; // snap to top
                    dyn->velocityY = 0.0f;
                    dyn->grounded = true;
                }

                // Skip horizontal and head-hit resolution for one-way platforms
                A->OnCollision(B);
                B->OnCollision(A);
                continue;
            }

            // Normal 2D AABB resolution (non one-way)
            if (inter.h <= inter.w) {
                // Vertical resolution
                if (dyn->velocityY > 0 && (Db.y + Db.h) > Sb.y && Db.y < Sb.y) {
                    dyn->y -= inter.h; dyn->velocityY = 0.0f; dyn->grounded = true;
                } else if (dyn->velocityY < 0 && Db.y < (Sb.y + Sb.h) && (Db.y + Db.h) > (Sb.y + Sb.h)) {
                    dyn->y += inter.h; dyn->velocityY = 0.0f;
                } else {
                    // fallback by centers
                    if ((Db.y + Db.h * 0.5f) < (Sb.y + Sb.h * 0.5f)) {
                        dyn->y -= inter.h; dyn->velocityY = 0.0f; dyn->grounded = true;
                    } else {
                        dyn->y += inter.h; dyn->velocityY = 0.0f;
                    }
                }
            } else {
                // Horizontal resolution
                bool fromLeft = (Db.x + Db.w * 0.5f) < (Sb.x + Sb.w * 0.5f);
                dyn->x += fromLeft ? -inter.w : inter.w;
                // optional: dyn->velocityX = 0.0f;
            }

            A->OnCollision(B);
            B->OnCollision(A);
        }
    }
}