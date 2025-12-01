#pragma once
#include <iostream>

#include "Core/GameEngine.h"
// #include <memory>

class TestEntity : public Entity {
  inline static MemoryPool* TestEntityMemoryPool = nullptr;
 public:
  void* operator new(size_t size) {
    if (!TestEntity::TestEntityMemoryPool)
      TestEntity::TestEntityMemoryPool = new MemoryPool(sizeof(TestEntity), 128);
    int sl_id = TestEntity::TestEntityMemoryPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return TestEntity::TestEntityMemoryPool->getPtr(sl_id);
  }

  void operator delete(void* ptr) {
    TestEntity::TestEntityMemoryPool->freeSlot(TestEntity::TestEntityMemoryPool->getSlot(ptr));
  }

  TestEntity(float x, float y, Timeline *tl, SDL_Renderer *renderer) : Entity(x, y, 128, 128, tl) {
    EnablePhysics(true);
    EnableCollision(false, false);
    SetVelocity(0.0f, 0.0f);
    SetCurrentFrame(0);
    
    // Initialize components
    setComponent("lastFrameTime", static_cast<Uint32>(0));
    setComponent("animationDelay", 200);
    setComponent("grounded", false);
    setComponent("wasGrounded", false);
    setComponent("groundRef", static_cast<Entity*>(nullptr));
    setComponent("playerInputDirection", 0); // -1 for left, 0 for idle, 1 for right
    
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
    (void)entitySpawner;
    
    // Update animation
    Uint32 lastFrameTime = getComponent<Uint32>("lastFrameTime");
    int animationDelay = getComponent<int>("animationDelay");
    lastFrameTime += (Uint32)(deltaTime * 1000);  // Convert to milliseconds
    if (lastFrameTime >= (Uint32)animationDelay) {
      rendering.currentFrame = (rendering.currentFrame + 1) % rendering.textures[rendering.currentTextureState].num_frames_x;
      lastFrameTime = 0;
    }
    setComponent("lastFrameTime", lastFrameTime);

    // Reset grounded state each frame (will be set by collision if on platform)
    setComponent("grounded", false);
    
    // Bounce off screen edges (demonstrates entity system working) using window
    // bounds push opposite direction
    if (position.x <= 0) {
      position.x = 0;
    }

    // Reset if falls off bottom (demonstrates physics working)
    if (position.y > 1080) { // fell off bottom of screen
      position.x = 100;
      position.y = 100;
      SetVelocityY(0.0f);
      setComponent("grounded", false);
      setComponent("groundRef", static_cast<Entity*>(nullptr));
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

    // Speed up and slow down the timeline for this entity
    static bool iKeyWasPressed = false;
    static bool oKeyWasPressed = false;
    static bool uKeyWasPressed = false;
    bool iKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_I);
    bool oKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_O);
    bool uKeyIsPressed = input->IsKeyPressed(SDL_SCANCODE_U);
    if (iKeyIsPressed && !iKeyWasPressed) {
      timeline->setScale(timeline->getScale() - 0.5f);
    }
    if (oKeyIsPressed && !oKeyWasPressed) {
      timeline->setScale(timeline->getScale() + 0.5f);
    }
    if (uKeyIsPressed && !uKeyWasPressed) {
      timeline->setScale(0.5f);
    }
    iKeyWasPressed = iKeyIsPressed;
    oKeyWasPressed = oKeyIsPressed;
    uKeyWasPressed = uKeyIsPressed;
  }

  void OnActivity(const std::string& actionName) override {
    // speeds
    constexpr float runSpeed = 200.0f;
    
    // Get ground reference and grounded state
    Entity* groundRef = getComponent<Entity*>("groundRef");
    bool grounded = getComponent<bool>("grounded");
    SDL_Log("OnActivity: actionName: %s", actionName.c_str());
    if (actionName == "MOVE_LEFT") {
      // Move left at constant speed, ignoring platform motion
      SetVelocityX(-runSpeed);
      setComponent("playerInputDirection", -1);
    } else if (actionName == "MOVE_RIGHT") {
      // Move right at constant speed, ignoring platform motion
      SetVelocityX(runSpeed);
      setComponent("playerInputDirection", 1);
    } else if (actionName == "JUMP") {
      // Only jump if grounded, but don't reset horizontal velocity if not grounded
      if (grounded) {
        SetVelocityY(-1500.0f);
        setComponent("grounded", false);
        setComponent("wasGrounded", false);
      }
      // If not grounded, do nothing - preserve current horizontal velocity
    } else if (actionName == "IDLE") {
      // Stop horizontal movement, inherit platform velocity when grounded
      const float carrierVX = (grounded && groundRef) ? groundRef->GetVelocityX() : 0.0f;
      SetVelocityX(carrierVX);
      setComponent("playerInputDirection", 0);
    } else {
      // Default case - also treat as IDLE
      const float carrierVX = (grounded && groundRef) ? groundRef->GetVelocityX() : 0.0f;
      SetVelocityX(carrierVX);
      setComponent("playerInputDirection", 0);
    }
  }

  void OnCollision(Entity *other, CollisionData *collData) override {
    if (collData->normal.y == -1.0f && collData->normal.x == 0.0f) {
      bool wasGrounded = getComponent<bool>("wasGrounded");
      if (!wasGrounded) {
        setComponent("wasGrounded", true);
      }
      setComponent("grounded", true);
      SetVelocityY(0.0f);
      setComponent("groundRef", other);
    } else if (collData->normal.x != 0.0f && other->entityType != "ScrollBoundary") {
      // Only stop horizontal movement for non-ScrollBoundary collisions
      SetVelocityX(0.0f);
    }
  }

  // Get current frame for rendering
  bool GetSourceRect(SDL_FRect &out) const override {
    out = SampleTextureAt(rendering.currentFrame, 0);
    return true;
  }
};
class Platform : public Entity {
 inline static MemoryPool* PlatformMemoryPool = nullptr;
 public:
  void* operator new(size_t size) {
    if (!Platform::PlatformMemoryPool)
      Platform::PlatformMemoryPool = new MemoryPool(sizeof(TestEntity), 128);
    int sl_id = Platform::PlatformMemoryPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return Platform::PlatformMemoryPool->getPtr(sl_id);
  }

  void operator delete(void* ptr) {
    Platform::PlatformMemoryPool->freeSlot(Platform::PlatformMemoryPool->getSlot(ptr));
  }

  Platform(float x, float y, float w = 200, float h = 20, bool moving = false, Timeline *tl = nullptr, SDL_Renderer *renderer = nullptr)
      : Entity(x, y, w, h, tl) {
    entityType = "Platform";
    EnableCollision(false, true);  // not a ghost, is kinematic
    if (moving) {
      EnablePhysics(false);  // Enable physics but no gravity
      SetVelocity(-100.0f, 0.0f);
    }
    if (renderer) {
      SDL_Texture *platformTexture =
      LoadTexture(renderer,
                  "media/cartooncrypteque_platform_basicground_idle.bmp");
      if (platformTexture) {
        rendering.textures[0] = {
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
    if(!physicsEnabled) return;
    // Horizontal-only motion for the moving platform
    position = add(position, mul(dt, {GetVelocityX(), GetVelocityY()}));
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
    SetVelocityX(-GetVelocityX());
  }
};

class ScrollBoundary : public Entity {
 inline static MemoryPool* ScrollBoundaryMemoryPool = nullptr;
 public:
  void* operator new(size_t size) {
    if (!ScrollBoundary::ScrollBoundaryMemoryPool)
      ScrollBoundary::ScrollBoundaryMemoryPool = new MemoryPool(sizeof(TestEntity), 128);
    int sl_id = ScrollBoundary::ScrollBoundaryMemoryPool->alloc();
    if (sl_id == -1)
      return nullptr;
    return ScrollBoundary::ScrollBoundaryMemoryPool->getPtr(sl_id);
  }

  void operator delete(void* ptr) {
    ScrollBoundary::ScrollBoundaryMemoryPool->freeSlot(ScrollBoundary::ScrollBoundaryMemoryPool->getSlot(ptr));
  }

  ScrollBoundary(float x, float y, float w, float h, Timeline *tl = nullptr, SDL_Renderer *renderer = nullptr, float maxOffsetX = 0.0f)
      : Entity(x, y, w, h, tl) {
    entityType = "ScrollBoundary";
    EnableCollision(true, false);
    setComponent("enabledScroll", false);
    SetVisible(false);
    setComponent("maxOffsetX", maxOffsetX);
  }

  void OnCollision(Entity *other, CollisionData *collData) override {
    if(collData->normal.x != 0.0f && other->entityType == "TestEntity" && other->GetOffSetX() >= getComponent<float>("maxOffsetX")) {
      std::cout<<"OnCollision: enabledScroll: "<<getComponent<bool>("enabledScroll")<<std::endl;
      other->SetOffSetX(other->GetOffSetX() - 900.0f);
      setComponent("enabledScroll", true);
    }
  }

  void Update(float dt, InputManager *input,
              EntityManager *entitySpawner) override {
    (void)input;
    (void)dt;
  }

};
