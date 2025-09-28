// GameEngine.h
#pragma once
#include <SDL3/SDL.h>

#include <memory>

#include "Collisions.h"
#include "Entity.h"
#include "Input.h"
#include "JobSystem.h"
#include "Physics.h"
#include "Render.h"
#include "Timeline.h"
#include <vector>


// Forward declarations

// Core Engine Class
class GameEngine {
 private:
  int winsizeX;
  int winsizeY;
  SDL_Window *window;
  SDL_Renderer *renderer;
  bool running;
  float tickRate;

  std::unique_ptr<PhysicsSystem> physics;
  std::unique_ptr<InputManager> input;
  std::unique_ptr<CollisionSystem> collision;
  std::unique_ptr<RenderSystem> renderSystem;
  std::unique_ptr<Timeline> rootTimeline;
  std::unique_ptr<EntityManager> entityManager;
  JobSystem jobSystem;


 public:
  GameEngine();
  ~GameEngine();

  bool Initialize(const char *title, int resx, int resy, float timeScale);
  void Run();
  void Shutdown();
  void Render(std::vector<Entity *> &);
  void Update(float deltaTime, std::vector<Entity *> &);
  void UpdateSystemsParallel(float deltaTime);
  EntityManager *GetEntityManager() { return entityManager.get(); }

  // void AddEntity(Entity *entity);
  // void RemoveEntity(Entity *entity);

  PhysicsSystem *GetPhysics() const { return physics.get(); }
  InputManager *GetInput() const { return input.get(); }
  CollisionSystem *GetCollision() const { return collision.get(); }
  RenderSystem *GetRenderSystem() const { return renderSystem.get(); }
  SDL_Renderer *GetRenderer() const { return renderer; }

  Timeline *GetRootTimeline() const { return rootTimeline.get(); }

private:
  void HandleEvents();
};
