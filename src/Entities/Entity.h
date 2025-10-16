#pragma once
#include "Timeline/Timeline.h"
#include <SDL3/SDL.h>
#include <map>
#include <string>

#include <vector>
#include <variant>

#include "Input/Input.h"
#include "Math/vec2.h"

class EntityManager;
class Entity;

typedef struct CollisionData {
  vec2 point;
  vec2 normal;
} CollisionData;

typedef struct Texture {
  SDL_Texture *sheet;
  uint32_t num_frames_x;
  uint32_t num_frames_y;
  uint32_t frame_width;
  uint32_t frame_height;
  bool loop;
} Texture;

typedef struct Physics {
  bool affectedByGravity = false;
  vec2 velocity;
  vec2 force;
} PhysicsComponent;

typedef struct RenderComponent {
  bool isVisible = true;
  std::map<int, Texture> textures;
  int currentTextureState = 0;
  int currentFrame = 0;
} RenderComponent;

typedef struct CollisionComponent {
  bool ghostEntity;
  bool isKinematic = false;
} CollisionComponent;

using Component = std::variant<int, 
float, 
bool, 
std::string, 
vec2, 
PhysicsComponent, 
Uint32,
Entity*,
CollisionComponent>;

class Entity {
 private:
  inline static int nextId =
      0;  // <-- inline variable: defined once program-wide
  int id;

 protected:
  std::map<std::string, Component> components;

 public:
  std::string entityType;

  vec2 position;
  vec2 dimensions;  

  bool physicsEnabled;

  bool collisionEnabled;
  
  Timeline *timeline = nullptr;

  RenderComponent rendering;
  
  // bool isStatic = false;  // Renamed to isKinematic in CollisionComponent struct

  virtual bool GetSourceRect(SDL_FRect &out) const {
    (void)out;
    return false;
  }

  void SetDimensions(float w, float h) {
    this->dimensions.x = w;
    this->dimensions.y = h;
  }

  void SetCurrentFrame(int frame) {
    rendering.currentFrame = frame;
  }

  void SetVisible(bool visible) {
    rendering.isVisible = visible;
  }

  Entity(float startX = 0.0f, float startY = 0.0f, float w = 32.0f,
         float h = 32.0f, Timeline *tl = nullptr)
      : id(nextId++), position({.x = startX, .y = startY}), dimensions({.x = w, .y = h}) {
    timeline = tl;
  }
  virtual ~Entity() = default;

  int GetId() const { return id; }
  void SetId(int id) { this->id = id; }

  void EnableCollision(bool ghostEntity = false, bool isKinematic = true) {
    collisionEnabled = true;
    setComponent("collision", CollisionComponent{
      .ghostEntity = ghostEntity,
      .isKinematic = isKinematic
    });
  }

  void SetGhostEntity(bool ghostEntity) {
    if(collisionEnabled) {
      getComponent<CollisionComponent>("collision").ghostEntity = ghostEntity;
    }
  }

  void SetIsKinematic(bool isKinematic) {
    if(collisionEnabled) {
      getComponent<CollisionComponent>("collision").isKinematic = isKinematic;
    }
  }

  void EnablePhysics(bool affectedByGravity = true) {
    physicsEnabled = true;
    setComponent("physics", PhysicsComponent{
      .affectedByGravity = affectedByGravity,
      .velocity = {0.0f, 0.0f},
      .force = {0.0f, affectedByGravity ? 9.8f * 300.0f : 0.0f}
    });
  }

  void SetAffectedByGravity(bool affectedByGravity) {
    if(physicsEnabled) {
      getComponent<PhysicsComponent>("physics").affectedByGravity = affectedByGravity;
      getComponent<PhysicsComponent>("physics").force.y = affectedByGravity ? 9.8f * 300.0f : 0.0f;
    }
  }
  
  void SetVelocity(float x, float y) {
    if(physicsEnabled) {
      getComponent<PhysicsComponent>("physics").velocity = {x, y};
    }
  }

  void SetVelocityX(float x) {
    if(physicsEnabled) {
      getComponent<PhysicsComponent>("physics").velocity.x = x;
    }
  }

  void SetVelocityY(float y) {
    if(physicsEnabled) {
      getComponent<PhysicsComponent>("physics").velocity.y = y;
    }
  }

  float GetVelocityX() {
    if(physicsEnabled) {
      return getComponent<PhysicsComponent>("physics").velocity.x;
    }
    return 0.0f;
  }

  float GetVelocityY() {
    if(physicsEnabled) {
      return getComponent<PhysicsComponent>("physics").velocity.y;
    }
    return 0.0f;
  }

  void SetForce(float x, float y) {
    if(physicsEnabled) {
      getComponent<PhysicsComponent>("physics").force = {x, y};
    }
  }

  virtual void Update(float, InputManager *, EntityManager *) {}
  virtual void OnActivity(const std::string&) {}
  virtual void OnCollision(Entity *, CollisionData *) {}


  bool hasComponent(const std::string& key) const {
    return components.find(key) != components.end();
  }
  
  void setComponent(const std::string& key, Component value) {
    components[key] = value;
  } 
 

  template <typename T>
  T& getComponent(const std::string& key) {
      return std::get<T>(components.at(key));
  }

  void SetTexture(int state, Texture *tex) {
    rendering.textures[state] = *tex;
    if(rendering.textures.size() == 1) {
      rendering.currentTextureState = state;
    }
  }

  void SetTextureState(int state) {
    if(rendering.textures.find(state) != rendering.textures.end()) {
      rendering.currentTextureState = state;
    }

  }

  inline SDL_FRect GetBounds() const {
    return SDL_FRect{position.x, position.y, dimensions.x, dimensions.y};
  }

  inline void SetPosition(float newX, float newY) {
    position = {.x = newX, .y = newY};
  }
  
  SDL_FRect SampleTextureAt(int x, int y) const {
    auto it = rendering.textures.find(rendering.currentTextureState);
    if (it == rendering.textures.end()) {
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

class EntityManager {
  std::vector<Entity *> entities;

 public:
  EntityManager() {}
  void AddEntity(Entity *entity);
  void RemoveEntity(Entity *entity);
  void ClearAllEntities();
  std::vector<Entity *> &getEntityVectorRef() { return entities; };
};

