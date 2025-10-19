#pragma once
#include <iostream>

#include "Core/GameEngine.h"
// #include <memory>

SDL_Texture *CreateColoredTexture(SDL_Renderer *renderer, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b) {
  SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);
  if (!surface) {
      SDL_Log("Failed to create surface: %s", SDL_GetError());
      return nullptr;
  }

  // Map RGBA color to the surface format
  Uint32 mappedColor = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), nullptr, r, g, b, 255);

  // Fill the surface with the specified color
  if (SDL_FillSurfaceRect(surface, nullptr, mappedColor) == false) {
      SDL_Log("Failed to fill surface: %s", SDL_GetError());
      SDL_DestroySurface(surface);
      return nullptr;
  }

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture) {
    return nullptr;
  }

  return texture;
}

class TestEntityBare : public Entity {
public:
  TestEntityBare(float x, float y, Timeline *tl, SDL_Renderer *renderer, std::string pTypeId) : Entity(x, y, 128, 128, tl) {
    EnablePhysics(false);
    EnableCollision(true, false);
    SetVelocity(0.0f, 0.0f);
    SetCurrentFrame(0);
    vec2 v = {.x = 0.0, .y = 0.0 };
    setComponent("V", v);

    entityType = pTypeId;
    SDL_Texture *etex = CreateColoredTexture(renderer, 512, 512, 128, 0, 0);
    std::cout<<"\nNNNNtex ninit\n";

    if (etex) {
      std::cout<<"tex init\n";
      Texture tex = {
        .sheet = etex,
        .num_frames_x = 1,
        .num_frames_y = 0,
        .frame_width = 512,
        .frame_height = 512,
        .loop = true
      };
      SetTexture(0, &tex);
    }
  }


  void Update(float dt, InputManager* im, EntityManager* em) override {

    if (position.x < 0.0)
      position.x = 999.0f;
    position.x = (float)((uint32_t)position.x % 1000);
    if (position.y < 0.0)
      position.y = 999.0f;
    position.y = (float)((uint32_t)position.y % 1000);
  }


  void OnActivity(const std::string& activity) override {
    // PhysicsComponent& phys = getComponent<PhysicsComponent>("physics");
    
    if (activity == "MOVE_LEFT") {
      getComponent<vec2>("V").x = -400.0f;
    }
    if (activity == "MOVE_RIGHT") {
      getComponent<vec2>("V").x = +400.0f;
    }
    if (activity=="MOVE_UP") {
      getComponent<vec2>("V").y = -400.0f;
    }
    if (activity=="MOVE_DOWN") {
      getComponent<vec2>("V").y = +400.0f;
    }
    if (activity == "") {
      getComponent<vec2>("V").x = 0.0f;
      getComponent<vec2>("V").y = 0.0f;

    }
    getComponent<PhysicsComponent>("physics").velocity = getComponent<vec2>("V");
  }
  
  bool GetSourceRect(SDL_FRect &out) const override {
    out = SampleTextureAt(0, 0);
    return true;
  }
};

// class TestEntity : public Entity {
//  public:
//   TestEntity(float x, float y, Timeline *tl, SDL_Renderer *renderer) : Entity(x, y, 128, 128, tl) {
//     EnablePhysics(true);
//     EnableCollision(false, false);
//     SetVelocity(0.0f, 0.0f);
//     SetCurrentFrame(0);
    
//     // Initialize components
//     setComponent("lastFrameTime", static_cast<Uint32>(0));
//     setComponent("animationDelay", 200);
//     setComponent("grounded", false);
//     setComponent("wasGrounded", false);
//     setComponent("groundRef", static_cast<Entity*>(nullptr));
    
//     entityType = "TestEntity";
//     SDL_Texture *entityTexture = LoadTexture(
//       renderer,
//       "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
//     if (entityTexture) {
//       Texture tex = {
//         .sheet = entityTexture,
//         .num_frames_x = 8,
//         .num_frames_y = 0,
//         .frame_width = 512,
//         .frame_height = 512,
//         .loop = true
//       };
//       SetTexture(0, &tex);
//     }
//   }

//   void Update(float deltaTime, InputManager *input,
//               EntityManager *entitySpawner) override {
//     (void)entitySpawner;
    
//     // Update animation
//     Uint32 lastFrameTime = getComponent<Uint32>("lastFrameTime");
//     int animationDelay = getComponent<int>("animationDelay");
//     lastFrameTime += (Uint32)(deltaTime * 1000);  // Convert to milliseconds
//     if (lastFrameTime >= (Uint32)animationDelay) {
//       rendering.currentFrame = (rendering.currentFrame + 1) % rendering.textures[rendering.currentTextureState].num_frames_x;
//       lastFrameTime = 0;
//     }
//     setComponent("lastFrameTime", lastFrameTime);

//     // speeds
//     constexpr float runSpeed = 200.0f;

//     // input
//     const bool left = input->IsKeyPressed(SDL_SCANCODE_A) ||
//                       input->IsKeyPressed(SDL_SCANCODE_LEFT);
//     const bool right = input->IsKeyPressed(SDL_SCANCODE_D) ||
//                        input->IsKeyPressed(SDL_SCANCODE_RIGHT);

//     // Get ground reference and grounded state
//     Entity* groundRef = getComponent<Entity*>("groundRef");
//     bool grounded = getComponent<bool>("grounded");
    
//     // carrier velocity (only meaningful when grounded on a platform)
//     const float carrierVX =
//         (grounded && groundRef) ? groundRef->GetVelocityX() : 0.0f;

//     // base desired velocity from input (world-space)
//     float desiredVX = 0.0f;
//     if (left ^ right) { // exactly one is held
//       desiredVX = left ? -runSpeed : runSpeed;
//     }

//     // Set velocity directly based on input state
//     if (desiredVX != 0.0f) {
//       SetVelocityX(desiredVX); // ignore platform motion while moving
//     } else {
//       SetVelocityX(carrierVX); // inherit when idle
//     }

//     // Jump handling
//     if (input->IsKeyPressed(SDL_SCANCODE_SPACE) && grounded) {
//       SetVelocityY(-1500.0f);
//       setComponent("grounded", false);
//       setComponent("wasGrounded", false);
//     }

//     // Bounce off screen edges (demonstrates entity system working) using window
//     // bounds push opposite direction
//     if (position.x <= 0) {
//       position.x = 0;
//     } else if (position.x + dimensions.x >= 1920) {
//       position.x = 1920 - dimensions.x;
//     }

//     // Reset if falls off bottom (demonstrates physics working)
//     if (!grounded) { // however you detect "no ground this frame"
//       setComponent("groundRef", static_cast<Entity*>(nullptr));
//     }
//     if (position.y > 1080) { // fell off bottom of screen
//       position.x = 100;
//       position.y = 100;
//       SetVelocityY(0.0f);
//       setComponent("grounded", false);
//       setComponent("groundRef", static_cast<Entity*>(nullptr));
//     }

//     // Handle pause toggle (only on key press, not while held)
//     static bool pKeyWasPressed = false;
//     bool pKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_P);
    
//     if (pKeyIsPressed && !pKeyWasPressed) {
//       // Key was just pressed (not held)
//       if (timeline->getState() == Timeline::State::PAUSE) {
//         timeline->setState(Timeline::State::RUN);
//       } else {
//         timeline->setState(Timeline::State::PAUSE);
//       }
//     }
//     pKeyWasPressed = pKeyIsPressed;

//     // Speed up and slow down the timeline for this entity
//     static bool iKeyWasPressed = false;
//     static bool oKeyWasPressed = false;
//     static bool uKeyWasPressed = false;
//     bool iKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_I);
//     bool oKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_O);
//     bool uKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_U);
//     if (iKeyIsPressed && !iKeyWasPressed) {
//       timeline->setScale(timeline->getScale() - 0.5f);
//     }
//     if (oKeyIsPressed && !oKeyWasPressed) {
//       timeline->setScale(timeline->getScale() + 0.5f);
//     }
//     if (uKeyIsPressed && !uKeyWasPressed) {
//       timeline->setScale(0.5f);
//     }
//     iKeyWasPressed = iKeyIsPressed;
//     oKeyWasPressed = oKeyIsPressed;
//     uKeyWasPressed = uKeyIsPressed;
//   }

//   void OnCollision(Entity *other, CollisionData *collData) override {
//     if (collData->normal.y == -1.0f && collData->normal.x == 0.0f) {
//       bool wasGrounded = getComponent<bool>("wasGrounded");
//       if (!wasGrounded) {
//         setComponent("wasGrounded", true);
//       }
//       setComponent("grounded", true);
//       SetVelocityY(0.0f);
//       setComponent("groundRef", other);
//     } else if (collData->normal.x != 0.0f) {
//       SetVelocityX(0.0f);
//     }
//   }

//   // Get current frame for rendering
//   bool GetSourceRect(SDL_FRect &out) const override {
//     out = SampleTextureAt(rendering.currentFrame, 0);
//     return true;
//   }
// };

// class Platform : public Entity {
//  public:
//   Platform(float x, float y, float w = 200, float h = 20, bool moving = false, Timeline *tl = nullptr, SDL_Renderer *renderer = nullptr)
//       : Entity(x, y, w, h, tl) {
//     entityType = "Platform";
//     EnableCollision(false, true);  // not a ghost, is kinematic
//     if (moving) {
//       EnablePhysics(false);  // Enable physics but no gravity
//       SetVelocity(-100.0f, 0.0f);
//     }
//     if (renderer) {
//       SDL_Texture *platformTexture =
//       LoadTexture(renderer,
//                   "media/cartooncrypteque_platform_basicground_idle.bmp");
//       if (platformTexture) {
//         rendering.textures[0] = {
//           .sheet = platformTexture,
//           .num_frames_x = 1,
//           .num_frames_y = 1,
//           .frame_width = 200,
//           .frame_height = 20,
//           .loop = true
//         };
//       }
//     }
//   }

//   void Update(float dt, InputManager *input,
//               EntityManager *entitySpawner) override {
//     (void)input;
//     (void)entitySpawner;  
//     if(!physicsEnabled) return;
//     // Horizontal-only motion for the moving platform
//     position = add(position, mul(dt, {GetVelocityX(), GetVelocityY()}));
//     if (position.x < 0) {
//       position.x = 0;
//       LeftRightOccilate(this);
//     } else if (position.x + dimensions.x > 1920) {
//       position.x = 1920 - dimensions.x;
//       LeftRightOccilate(this);
//     }
//   }

//   void LeftRightOccilate(Entity *other) {
//     (void)other;
//     // multiply the xvelocity by -1 to reverse direction
//     SetVelocityX(-GetVelocityX());
//   }
// };