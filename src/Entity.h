#pragma once
#include <Input.h>
#include <SDL3/SDL.h>
#include <vec2.h>

typedef struct CollisionData {
  vec2 point;
  vec2 normal;
} CollisionData;

class Entity {
private:
  inline static int nextId =
      0; // <-- inline variable: defined once program-wide
  int id;

public:
  vec2 position;
  vec2 prevPosition;
  vec2 dimensions; //
  vec2 velocity;   // float velocityX = 0.0f, velocityY = 0.0f;
  vec2 force;

  SDL_Texture *texture = nullptr;
  bool isVisible = true;

  bool hasPhysics = true;
  bool affectedByGravity = true;
  bool isStatic = false;
  bool grounded = false;
  bool isOneWay = false;

  virtual bool GetSourceRect(SDL_FRect &out) const { return false; }

  Entity(float startX = 0.0f, float startY = 0.0f, float w = 32.0f,
         float h = 32.0f)
      : id(nextId++), position({.x = startX, .y = startY}),
        prevPosition(position), dimensions({.x = w, .y = h}) {
    if (affectedByGravity) {
      force.y = 9.8 * 512.0;
    }
  }
  virtual ~Entity() = default;

  int GetId() const { return id; }

  virtual void Update(float, InputManager *) {}
  virtual void OnCollision(Entity *, CollisionData *) {}

  inline SDL_FRect GetBounds() const {
    return SDL_FRect{position.x, position.y, dimensions.x, dimensions.y};
  }
  inline SDL_FRect GetPrevBounds() const {
    return SDL_FRect{prevPosition.x, prevPosition.y, dimensions.x,
                     dimensions.y};
  }

  inline void SetPosition(float newX, float newY) {
    position = {.x = newX, .y = newY};
  }
  inline void SetTexture(SDL_Texture *tex) { texture = tex; }
  inline SDL_Texture *GetTexture() const { return texture; }
};
