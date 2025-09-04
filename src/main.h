#pragma once
#include "GameEngine.h"
// #include <memory>

class TestEntity : public Entity {
private:
  int currentFrame;
  Uint32 lastFrameTime;
  int animationDelay;
  int frameCount;
  int frameWidth;
  int frameHeight;

public:
  TestEntity(float x, float y) : Entity(x, y, 128, 128) {
    velocity.x = 150.0f; // Move right at 150 pixels per second
    currentFrame = 0;
    lastFrameTime = 0;
    animationDelay = 200;
    frameCount = 8;
    frameWidth = 512;
    frameHeight = 512;
  }

  void Update(float deltaTime, InputManager *input) override {
    // Update animation
    lastFrameTime += (Uint32)(deltaTime * 1000); // Convert to milliseconds
    if (lastFrameTime >= (Uint32)animationDelay) {
      currentFrame = (currentFrame + 1) % frameCount;
      lastFrameTime = 0;
    }

    velocity.x = 0.0;
    if (input->IsKeyPressed(SDL_SCANCODE_A) ||
        input->IsKeyPressed(SDL_SCANCODE_LEFT))
      velocity.x = -200.0f; // move left
    if (input->IsKeyPressed(SDL_SCANCODE_D) ||
        input->IsKeyPressed(SDL_SCANCODE_RIGHT))
      velocity.x = +200.0f; // move right

    if (input->IsKeyPressed(SDL_SCANCODE_SPACE) && grounded) {
      velocity.y = -1000.0f;
      grounded = false;
    }

    // Bounce off screen edges (demonstrates entity system working) using window bounds push opposite direction
    if (position.x <= 0) {
      position.x = 0;
    } else if (position.x + dimensions.x >= 1920) {
      position.x = 1920 - dimensions.x;
    }

    // Reset if falls off bottom (demonstrates physics working)
    if (position.y > 1080) {
      position.y = 100;
      velocity.y = 0;
    }
  }

  void OnCollision(Entity *other, CollisionData *collData) override {
    velocity.y = 0.0f; // Bounce response
  }

  // Get current frame for rendering
  bool GetSourceRect(SDL_FRect &out) const override {
    out.x = static_cast<float>(currentFrame * frameWidth);
    out.y = 0.0f;
    out.w = static_cast<float>(frameWidth);
    out.h = static_cast<float>(frameHeight);
    return true;
  }
};

class Platform : public Entity {
public:
  Platform(float x, float y, float w = 200, float h = 20, bool moving = false)
      : Entity(x, y, w, h) {
    isStatic = true;
    hasPhysics = false;
    affectedByGravity = false;
    isOneWay = true; // <- key line
    velocity.x = moving ? 200.0f : 0.0f;
    velocity.y = 0.0f;
  }

  void Update(float dt, InputManager *input) override {
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
    // multiply the xvelocity by -1 to reverse direction
    velocity.x *= -1;
  }
};