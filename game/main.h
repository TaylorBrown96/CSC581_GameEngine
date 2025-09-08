#pragma once
#include <vector>
#include <cmath>
#include "GameEngine.h"
#include "Config.h"

// ============================================================================
// Player (controllable, state-based animation; single-row spritesheets)
// ============================================================================
class Player : public Entity {
public:
  enum class Anim { Idle, Left, Right, Jump, Fall };

  Player(float x, float y)
  : Entity(x, y, cfg::PLAYER_WIDTH, cfg::PLAYER_HEIGHT),
    spawnX(x), spawnY(y) {
    hasPhysics = true;
    affectedByGravity = true;
    setAnimLayout(/*framesX*/1, /*frameW*/64, /*frameH*/64); // default; per-sheet overrides below
  }

  // Inject your sheets from main.cpp
  void SetSheets(SDL_Texture* idle, SDL_Texture* left, SDL_Texture* right,
                 SDL_Texture* jump, SDL_Texture* fall) {
    texIdle  = idle;
    texLeft  = left;
    texRight = right;
    texJump  = jump;
    texFall  = fall;
    applyAnim(Anim::Idle);
  }

  void Update(float dt, InputManager* input) override {
    // ---- Input flags
    const bool pressLeft  = input->IsKeyPressed(SDL_SCANCODE_A) || input->IsKeyPressed(SDL_SCANCODE_LEFT);
    const bool pressRight = input->IsKeyPressed(SDL_SCANCODE_D) || input->IsKeyPressed(SDL_SCANCODE_RIGHT);
    const bool pressJump  = input->IsKeyPressed(SDL_SCANCODE_SPACE);

    // ---- Horizontal movement
    float vx_input = 0.0f;
    if (pressLeft ^ pressRight) vx_input = pressLeft ? -cfg::PLAYER_SPEED_X : cfg::PLAYER_SPEED_X;

    // Always add platform motion while grounded on it (physics), but do NOT use this for animation
    float vx_platform = (grounded && carrier_) ? carrier_->velocity.x : 0.0f;
    velocity.x = vx_input + vx_platform;

    // ---- Jump
    if (pressJump && grounded) {
        velocity.y = cfg::PLAYER_JUMP_IMPULSE;
        grounded = false;
        carrier_ = nullptr;
    }

    // ---- Terminal velocity clamp (optional, if you added it earlier)
    const float MAX_FALL_SPEED = 1200.0f;
    if (velocity.y > MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;

    // ---- Animation selection: driven by INPUT, not resultant velocity
    Anim desired = Anim::Idle;
    if (!grounded) {
        desired = (velocity.y < 0.0f) ? Anim::Jump : Anim::Fall;
    } else if (pressLeft) {
        desired = Anim::Left;
    } else if (pressRight) {
        desired = Anim::Right;
    } else {
        desired = Anim::Idle; // on moving platform with no input -> stay idle
    }
    if (desired != currentAnim) applyAnim(desired);

    // ---- Advance animation frame
    lastFrameMS += static_cast<Uint32>(dt * 1000.f);
    if (lastFrameMS >= static_cast<Uint32>(animationDelayMS)) {
        lastFrameMS = 0;
        currentFrame = (currentFrame + 1) % std::max(1, framesX);
    }

    // ---- Bounds & death line (keep whatever you had)
    if (position.x < 0) position.x = 0;
    if (position.x + dimensions.x > cfg::SCREEN_WIDTH) position.x = cfg::SCREEN_WIDTH - dimensions.x;

    const float DEATH_Y = cfg::SCREEN_HEIGHT + 400.0f;
    if (position.y > DEATH_Y) Respawn();
  }

  void OnCollision(Entity* other, CollisionData* hit) override {
    // Landed from above
    if (hit->normal.y == -1.0f && std::abs(hit->normal.x) < 0.5f) {
      grounded = true;
      velocity.y = 0.f;
      carrier_ = other;
      return;
    }
    // Wall
    if (std::abs(hit->normal.x) > 0.5f) {
      velocity.x = 0.f;
    }
  }

  bool GetSourceRect(SDL_FRect& out) const override {
    out = SampleTextureAt(currentFrame, 0); // single row
    return true;
  }

private:
  // Save & restore spawn safely (resets velocity and flags)
  void Respawn() {
    position = {spawnX, spawnY};
    velocity = {0.f, 0.f};
    grounded = false;
    carrier_ = nullptr;
    currentFrame = 0;
    lastFrameMS = 0;
    // Reset to a sensible state/anim on spawn
    applyAnim(Anim::Idle);
  }

  // SINGLE-ROW spritesheet: engine expects num_frames_y = 0
  void setAnimLayout(int framesX_, int frameW, int frameH) {
    framesX = framesX_;
    tex.num_frames_x = framesX_;
    tex.num_frames_y = 0;     // <— critical for single-row sheets
    tex.frame_width  = frameW;
    tex.frame_height = frameH;
    currentFrame = 0;
    lastFrameMS  = 0;
    animationDelayMS = (framesX > 1) ? 120 : 250;
  }

  void applyAnim(Anim a) {
    currentAnim = a;
    switch (a) {
      case Anim::Idle:  if (texIdle)  SetTexture(texIdle);  setAnimLayout(1, 64, 64); break;
      case Anim::Left:  if (texLeft)  SetTexture(texLeft);  setAnimLayout(4, 64, 64); break; // 256x64 sheet
      case Anim::Right: if (texRight) SetTexture(texRight); setAnimLayout(4, 64, 64); break; // 256x64 sheet
      case Anim::Jump:  if (texJump)  SetTexture(texJump);  setAnimLayout(1, 64, 64); break;
      case Anim::Fall:  if (texFall)  SetTexture(texFall);  setAnimLayout(1, 64, 64); break;
    }
  }

  // Animation
  int     framesX = 1;
  int     currentFrame = 0;
  Uint32  lastFrameMS = 0;
  int     animationDelayMS = 120;

  // Sheets
  SDL_Texture* texIdle  = nullptr;
  SDL_Texture* texLeft  = nullptr;
  SDL_Texture* texRight = nullptr;
  SDL_Texture* texJump  = nullptr;
  SDL_Texture* texFall  = nullptr;

  // Platform carrier and spawn
  Entity* carrier_ = nullptr;
  float   spawnX   = 100.f;
  float   spawnY   = 100.f;

  Anim currentAnim = Anim::Idle;
};

// ============================================================================
// Ground (static collider)
// ============================================================================
class Ground : public Entity {
public:
  Ground(float x, float y, float w, float h) : Entity(x, y, w, h) {
    isStatic = true;
    hasPhysics = false;
    affectedByGravity = false;
    isOneWay = true;
  }
};

// ============================================================================
// MovingGround (bridge oscillates horizontally; acts as static collider)
// ============================================================================
class MovingGround : public Ground {
public:
  MovingGround(float x, float y, float w, float h, float distance, float speed)
  : Ground(x, y, w, h), startX(x), travelDist(distance), moveSpeed(speed) {
    direction = 1.f;
    velocity = {0.f, 0.f}; // ensure initialized
  }

  void Update(float dt, InputManager* /*in*/) override {
    const float prevX = position.x;

    // integrate manually
    position.x += direction * moveSpeed * dt;

    // reverse at bounds
    if (position.x > startX + travelDist) {
      position.x = startX + travelDist;
      direction = -1.f;
    } else if (position.x < startX) {
      position.x = startX;
      direction =  1.f;
    }

    // publish platform velocity for riders
    if (dt > 0.f) velocity.x = (position.x - prevX) / dt; else velocity.x = 0.f;
    velocity.y = 0.f;
  }

private:
  float startX, travelDist, moveSpeed, direction;
};

// -----------------------------
// PatrolBot (swirling orb — 4 frames, 128x128 each, in a single row)
// -----------------------------
class PatrolBot : public Entity {
public:
  PatrolBot(const std::vector<SDL_FPoint>& waypoints, float speed_px_s)
  : Entity(waypoints.empty()?0.f:waypoints[0].x,
           waypoints.empty()?0.f:waypoints[0].y,
           128.f, 128.f),   // size matches one frame
    path(waypoints), speed(speed_px_s) {
    isStatic = false;
    hasPhysics = false;
    affectedByGravity = false;
    setLayout(4, 128, 128); // 4 frames, each 128x128
  }

  void Update(float dt, InputManager* /*input*/) override {
    // Animate
    orbLastFrameMS += static_cast<Uint32>(dt * 1000.f);
    if (orbLastFrameMS >= 120) { // 120ms per frame
      orbLastFrameMS = 0;
      orbFrame = (orbFrame + 1) % framesX;
    }

    // Follow waypoints
    if (path.size() < 2) return;
    const SDL_FPoint& a = path[idx];
    const SDL_FPoint& b = path[(idx + 1) % path.size()];
    float dx = b.x - a.x, dy = b.y - a.y;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 1e-4f) { idx = (idx + 1) % path.size(); t = 0.f; return; }
    t += (speed * dt) / len;
    if (t >= 1.f) { t -= 1.f; idx = (idx + 1) % path.size(); }
    const SDL_FPoint& a2 = path[idx];
    const SDL_FPoint& b2 = path[(idx + 1) % path.size()];
    position.x = a2.x + (b2.x - a2.x) * t;
    position.y = a2.y + (b2.y - a2.y) * t;
  }

  void OnCollision(Entity* other, CollisionData*) override {
    if (auto* p = dynamic_cast<Player*>(other)) {
      p->position = {100.f,100.f};
      p->velocity = {0.f,0.f};
      p->grounded = false;
    }
  }

  bool GetSourceRect(SDL_FRect& out) const override {
    out = SampleTextureAt(orbFrame, 0); // row 0, single row
    return true;
  }

  void SetTextureAndLayout(SDL_Texture* t) {
    SetTexture(t);
    setLayout(4, 128, 128); // 4 frames across, each 128x128
  }

private:
  void setLayout(int framesX_, int frameW, int frameH) {
    framesX = framesX_;
    tex.num_frames_x = framesX_;
    tex.num_frames_y = 0;     // single row
    tex.frame_width  = frameW;
    tex.frame_height = frameH;
    orbFrame = 0;
    orbLastFrameMS = 0;
  }

  // Animation
  int     framesX = 4;
  int     orbFrame = 0;
  Uint32  orbLastFrameMS = 0;

  // Path following
  std::vector<SDL_FPoint> path;
  float        speed = 120.f;
  std::size_t  idx   = 0;
  float        t     = 0.f;
};
