#pragma once
#include <Input.h>
#include <SDL3/SDL.h>
#include <vec2.h>
#include <map>

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
  bool loop;
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

  std::map<int, Texture> textures;
  int currentTextureState = 0;
  
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
  }
  virtual ~Entity() = default;

  int GetId() const { return id; }

  virtual void Update(float, InputManager *) {}
  virtual void OnCollision(Entity *, CollisionData *) {}

  void SetTexture(int state, Texture *tex) {
    textures[state] = *tex;
    if(textures.size() == 1) {
      currentTextureState = state;
    }
  }

  void SetTextureState(int state) {
    if(textures.find(state) != textures.end()) {
      currentTextureState = state;
    }
  }

  inline SDL_FRect GetBounds() const {
    return SDL_FRect{position.x, position.y, dimensions.x, dimensions.y};
  }

  inline void SetPosition(float newX, float newY) {
    position = {.x = newX, .y = newY};
  }
  
  SDL_FRect SampleTextureAt(int x, int y) const {
    auto it = textures.find(currentTextureState);
    if (it == textures.end()) {
      return {.x = 0, .y = 0, .w = 0, .h = 0}; // Return empty rect if texture not found
    }
    const Texture& tex = it->second;
    return {
      .x = (float)(x * tex.frame_width),
      .y = (float)(y * tex.frame_height),
      .w = (float)(tex.frame_width),
      .h = (float)(tex.frame_height)
    };  
  }
};
