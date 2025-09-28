#pragma once
#include <iostream>

#include "GameEngine.h"
// #include <memory>

class TestEntity : public Entity {
 private:
  Uint32 lastFrameTime;
  int animationDelay;

  Entity *groundRef = nullptr;  // platform we're standing on (if any)
  float groundVX = 0.0f;        // platform's current x velocity

 public:
  TestEntity(float x, float y, Timeline *tl, SDL_Renderer *renderer) : Entity(x, y, 128, 128, tl) {
    velocity.x = 0.0f;  // Move right at 150 pixels per second
    currentFrame = 0;
    lastFrameTime = 0;
    animationDelay = 200;
    entityType = "TestEntity";
    SDL_Texture *entityTexture = LoadTexture(
      renderer,
      "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
    if (entityTexture) {
      Texture tex = {
        .sheet = entityTexture,
        .num_frames_x = 8,
        .num_frames_y = 0,
        .frame_width = 512,
        .frame_height = 512,
        .loop = true
      };
      SetTexture(0, &tex);
    }
  }

  void Update(float deltaTime, InputManager *input,
              EntityManager *entitySpawner) override {
    // Update animation
    (void) entitySpawner;
    lastFrameTime += (Uint32)(deltaTime * 1000);  // Convert to milliseconds
    if (lastFrameTime >= (Uint32)animationDelay) {
      currentFrame = (currentFrame + 1) % textures[currentTextureState].num_frames_x;
      lastFrameTime = 0;
    }

    // Handle platform motion inheritance when no movement input is active
    // This needs to be in Update because OnActivity is only called on button press
    const float carrierVX = (grounded && groundRef) ? groundRef->velocity.x : 0.0f;
    
    // Check if no movement input is currently active
    const bool left = input->IsKeyPressed(SDL_SCANCODE_A) || input->IsKeyPressed(SDL_SCANCODE_LEFT);
    const bool right = input->IsKeyPressed(SDL_SCANCODE_D) || input->IsKeyPressed(SDL_SCANCODE_RIGHT);
    
    // If no movement input is active, inherit platform motion
    // if (!left && !right) {
    //   velocity.x = carrierVX;  // inherit platform motion when idle
    // }

    // Bounce off screen edges (demonstrates entity system working) using window
    // bounds push opposite direction
    if (position.x <= 0) {
      position.x = 0;
    } else if (position.x + dimensions.x >= 1920) {
      position.x = 1920 - dimensions.x;
    }

    // Reset if falls off bottom (demonstrates physics working)
    if (!grounded) {  // however you detect “no ground this frame”
      groundRef = nullptr;
      groundVX = 0.0f;
    }
    if (position.y > 1080) {  // fell off bottom of screen
      position.x = 100;
      position.y = 100;
      velocity.y = 0.0f;
      grounded = false;
      groundRef = nullptr;
      groundVX = 0.0f;
    }
    

    // Handle pause toggle (only on key press, not while held)
    static bool pKeyWasPressed = false;
    bool pKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_P);
    
    if (pKeyIsPressed && !pKeyWasPressed) {
      // Key was just pressed (not held)
      if (timeline->getState() == Timeline::State::PAUSE) {
        timeline->setState(Timeline::State::RUN);
      } else {
        timeline->setState(Timeline::State::PAUSE);
      }
    }
    pKeyWasPressed = pKeyIsPressed;
  }

  void OnActivity(const std::string& actionName) override {
    // speeds
    constexpr float runSpeed = 200.0f;
    
    if (actionName == "MOVE_LEFT") {
      // Move left at constant speed, ignoring platform motion
      velocity.x = -runSpeed;
    } else if (actionName == "MOVE_RIGHT") {
      // Move right at constant speed, ignoring platform motion
      velocity.x = runSpeed;
    } else if (actionName == "JUMP" && grounded) {
      velocity.y = -1500.0f;
      grounded = false;
    } else {
      const float carrierVX = (grounded && groundRef) ? groundRef->velocity.x : 0.0f;
      velocity.x = carrierVX;
    }
  }

  void OnCollision(Entity *other, CollisionData *collData) override {
    if (collData->normal.y == -1.0f && collData->normal.x == 0.0f) {
      grounded = true;
      velocity.y = 0.0f;
      groundRef = other;
    } else if (collData->normal.x != 0.0f) {
      velocity.x =
          0.0f;  // or keep desiredVX if you resolve penetration separately
    }
  }

  // Get current frame for rendering
  bool GetSourceRect(SDL_FRect &out) const override {
    out = SampleTextureAt(currentFrame, 0);
    return true;
  }
};

class Platform : public Entity {
 public:
  Platform(float x, float y, float w = 200, float h = 20, bool moving = false, Timeline *tl = nullptr, SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    entityType = "Platform";
    isStatic = true;
    hasPhysics = false;
    affectedByGravity = false;
    velocity.x = moving ? -100.0f : 0.0f;
    velocity.y = 0.0f;
    if (renderer) {
      SDL_Texture *platformTexture =
      LoadTexture(renderer,
                  "media/cartooncrypteque_platform_basicground_idle.bmp");
      if (platformTexture) {
        textures[0] = {
          .sheet = platformTexture,
          .num_frames_x = 1,
          .num_frames_y = 1,
          .frame_width = 200,
          .frame_height = 20,
          .loop = true
        };
      }
    }
  }

  void Update(float dt, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)input;
    (void)entitySpawner;
    // Horizontal-only motion for the moving platform
    position = add(position, mul(dt, velocity));
    if (position.x < 0) {
      position.x = 0;
      LeftRightOccilate(this);
    } else if (position.x + dimensions.x > 1920) {
      position.x = 1920 - dimensions.x;
      LeftRightOccilate(this);
    }
  }

  void LeftRightOccilate(Entity *other) {
    (void)other;
    // multiply the xvelocity by -1 to reverse direction
    velocity.x *= -1;
  }
};