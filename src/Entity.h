#pragma once
#include <SDL3/SDL.h>

#include <Input.h>

class Entity {
private:
  inline static int nextId =
      0; // <-- inline variable: defined once program-wide
  int id;

public:
  float x = 0.0f, y = 0.0f;
  float prevX = 0.0f, prevY = 0.0f;
  float width = 32.0f, height = 32.0f;
  float velocityX = 0.0f, velocityY = 0.0f;
  float forceX = 0.0f, forceY = 0.0f;

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
      : id(nextId++), x(startX), y(startY), prevX(startX), prevY(startY),
        width(w), height(h) {
    if (affectedByGravity) {
      forceY = 9.8 * 512.0;
    }
  }
  virtual ~Entity() = default;

  int GetId() const { return id; }

  virtual void Update(float, InputManager*) {}
  virtual void OnCollision(Entity*) {}

  inline SDL_FRect GetBounds() const { return SDL_FRect{x, y, width, height}; }
  inline SDL_FRect GetPrevBounds() const {
    return SDL_FRect{prevX, prevY, width, height};
  }

  inline void SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
  }
  inline void SetTexture(SDL_Texture *tex) { texture = tex; }
  inline SDL_Texture *GetTexture() const { return texture; }
};
