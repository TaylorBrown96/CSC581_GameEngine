// GameEngine.h
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <unordered_map>

// Forward declarations
class Entity;
class PhysicsSystem;
class InputManager;
class CollisionSystem;
class RenderSystem;

// Core Engine Class
class GameEngine {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::unique_ptr<PhysicsSystem> physics;
    std::unique_ptr<InputManager> input;
    std::unique_ptr<CollisionSystem> collision;
    std::unique_ptr<RenderSystem> renderSystem;
    
    std::vector<std::shared_ptr<Entity>> entities;
    
public:
    GameEngine();
    ~GameEngine();
    
    bool Initialize();
    void Run();
    void Shutdown();
    
    void AddEntity(std::shared_ptr<Entity> entity);
    void RemoveEntity(std::shared_ptr<Entity> entity);
    
    PhysicsSystem* GetPhysics() const { return physics.get(); }
    InputManager* GetInput() const { return input.get(); }
    CollisionSystem* GetCollision() const { return collision.get(); }
    RenderSystem* GetRenderSystem() const { return renderSystem.get(); }
    SDL_Renderer* GetRenderer() const { return renderer; }
    
private:
    void Update(float deltaTime);
    void Render();
};

// Entity System - Generic game object
class Entity {
private:
    static int nextId;
    int id;
    
public:
    float x, y;           // Position
    float width, height;  // Dimensions
    float velocityX, velocityY; // Velocity for physics
    SDL_Texture* texture;
    bool hasPhysics;
    bool isVisible;
    
    Entity(float startX = 0, float startY = 0, float w = 32, float h = 32);
    virtual ~Entity();
    
    int GetId() const { return id; }
    virtual void Update(float deltaTime) {}
    virtual void OnCollision(Entity* other) {}
    
    // Bounding box for collision detection
    SDL_FRect GetBounds() const;
    void SetPosition(float newX, float newY);
    void SetTexture(SDL_Texture* tex);
};

// Physics System
class PhysicsSystem {
private:
    float gravity;
    
public:
    PhysicsSystem() : gravity(980.0f) {} // Default gravity (pixels/second^2)
    
    void SetGravity(float value) { gravity = value; }
    float GetGravity() const { return gravity; }
    
    void ApplyPhysics(Entity* entity, float deltaTime);
    void ApplyGravity(Entity* entity, float deltaTime);
};

// Input Manager
class InputManager {
private:
    const bool* keyboardState;  // SDL3 returns const bool*, not const Uint8*
    std::unordered_map<SDL_Scancode, bool> previousKeyState;
    
public:
    InputManager();
    void Update();
    
    bool IsKeyPressed(SDL_Scancode scancode) const;
    bool IsKeyJustPressed(SDL_Scancode scancode) const;
    bool IsKeyJustReleased(SDL_Scancode scancode) const;
};

// Collision Detection System
class CollisionSystem {
public:
    bool CheckCollision(const Entity* entity1, const Entity* entity2) const;
    bool CheckCollision(const SDL_FRect& rect1, const SDL_FRect& rect2) const;
    
    void ProcessCollisions(std::vector<std::shared_ptr<Entity>>& entities);
};

// Render System with Scaling
enum class ScalingMode {
    CONSTANT_SIZE,    // Pixel-based
    PROPORTIONAL     // Percentage-based
};

class RenderSystem {
private:
    SDL_Renderer* renderer;
    ScalingMode currentMode;
    float screenWidth, screenHeight;
    float baseWidth, baseHeight; // Reference resolution for proportional scaling
    
public:
    RenderSystem(SDL_Renderer* renderer);
    
    void SetScalingMode(ScalingMode mode);
    ScalingMode GetScalingMode() const { return currentMode; }
    void ToggleScalingMode();
    
    void RenderEntity(const Entity* entity);
    void RenderEntity(const Entity* entity, const SDL_FRect* sourceRect); // For sprite animation
    void SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void Clear();
    void Present();
    
private:
    SDL_FRect CalculateRenderRect(const Entity* entity);
};