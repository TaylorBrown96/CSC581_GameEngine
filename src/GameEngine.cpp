// GameEngine.cpp
// #include <memory>
#include "GameEngine.h"
#include <algorithm>
#include <Config.h>

// GameEngine Implementation
GameEngine::GameEngine() : window(nullptr), renderer(nullptr), running(false) {}

GameEngine::~GameEngine() { Shutdown(); }

bool GameEngine::Initialize(const char* title, int resx, int resy) {
  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return false;
  }

  // Create window (1920x1080 as required)
  winsizeX = resx;
  winsizeY = resy;
  window = SDL_CreateWindow(title, resx, resy, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return false;
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    SDL_Log("Failed to create renderer: %s", SDL_GetError());
    return false;
  }

  // Initialize systems
  physics = std::make_unique<PhysicsSystem>();
  input = std::make_unique<InputManager>();
  collision = std::make_unique<CollisionSystem>();
  renderSystem = std::make_unique<RenderSystem>(renderer, resx, resy);

  running = true;
  return true;
}

void GameEngine::Run() {
  SDL_Event event;
  Uint32 lastTime = SDL_GetTicks();

  while (running) {
    // Calculate delta time
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime);
    lastTime = currentTime;

    // Handle events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT || input->IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
      }   
    }
    // Update input
    input->Update();

    // Update game
    Update(deltaTime / 1000.0);

    // Render
    Render();

    float delay = std::max(0.0, 1000.0 / cfg::TARGET_FPS - deltaTime);
    SDL_Delay(delay);
  }
}

void GameEngine::Update(float deltaTime) {
  // Update all entities
  for (auto &entity : entities) {
    entity->Update(deltaTime, input.get());

    // Apply physics if entity has physics enabled
    if (entity->hasPhysics) {
      physics->ApplyPhysics(entity, deltaTime);
    }
  }

  // Process collisions
  collision->ProcessCollisions(entities);
}

void GameEngine::Render() {
  if (input->IsKeyPressed(SDL_SCANCODE_0)){
    renderSystem->SetScalingMode(ScalingMode::CONSTANT_SIZE);
  }
  if (input->IsKeyPressed(SDL_SCANCODE_9)){
    renderSystem->SetScalingMode(ScalingMode::PROPORTIONAL);
  }


  if (renderSystem->GetScalingMode() == ScalingMode::PROPORTIONAL) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    renderSystem->screenHeight = (float)h;
    renderSystem->screenWidth = (float)w;
  }
  // Clear screen to blue as required
  renderSystem->SetBackgroundColor(cfg::CLEAR_R, cfg::CLEAR_G, cfg::CLEAR_B, cfg::CLEAR_A);
  renderSystem->Clear();

  // Render all visible entities
  for (const auto &entity : entities) {
    if (entity->isVisible) {
      renderSystem->RenderEntity(entity);
    }
  }

  renderSystem->Present();
}

void GameEngine::AddEntity(Entity *entity) { entities.push_back(entity); }

void GameEngine::RemoveEntity(Entity *entity) {
  entities.erase(std::remove(entities.begin(), entities.end(), entity),
                 entities.end());
}

void GameEngine::Shutdown() {
  entities.clear();

  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  SDL_Quit();
}
