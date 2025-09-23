#pragma once
#include <Input.h>
#include <SDL3/SDL.h>
#include <vec2.h>

typedef struct CollisionData {
  vec2 point;
  vec2 normal;
} CollisionData;

typedef struct Texture {
  SDL_Texture* sheet;
  uint32_t num_frames_x;
  uint32_t num_frames_y;
  uint32_t frame_width;
  uint32_t frame_height;
} Texture;


class Entity {
private:
  inline static int nextId =
      0; // <-- inline variable: defined once program-wide
  int id;

public:
  vec2 position;
  vec2 dimensions; //
  vec2 velocity;   // float velocityX = 0.0f, velocityY = 0.0f;
  vec2 force;

  Texture tex;
  bool isVisible = true;

  bool hasPhysics = true;
  bool affectedByGravity = true;
  bool isStatic = false;
  bool grounded = false;

  virtual bool GetSourceRect(SDL_FRect &out) const { (void)out; return false; }

  Entity(float startX = 0.0f, float startY = 0.0f, float w = 32.0f,
         float h = 32.0f)
      : id(nextId++), position({.x = startX, .y = startY}), dimensions({.x = w, .y = h}) {
    if (affectedByGravity) {
      force.y = 9.8 * 300.0;
    }
    tex.sheet = nullptr;
  }
  virtual ~Entity() = default;

  int GetId() const { return id; }

  virtual void Update(float, InputManager *) {}
  virtual void OnCollision(Entity *, CollisionData *) {}

  inline SDL_FRect GetBounds() const {
    return SDL_FRect{position.x, position.y, dimensions.x, dimensions.y};
  }
  inline void SetPosition(float newX, float newY) {
    position = {.x = newX, .y = newY};
  }
  inline void SetTexture(SDL_Texture *texp) { tex.sheet = texp; }
  inline SDL_Texture *GetTexture() const { return tex.sheet; }
  
  SDL_FRect SampleTextureAt(int x, int y) const {
    return {
      .x = (float)(x * tex.frame_width),
      .y = (float)(y * tex.frame_height),
      .w = (float)(tex.frame_width),
      .h = (float)(tex.frame_height)
    };  
  }
};
