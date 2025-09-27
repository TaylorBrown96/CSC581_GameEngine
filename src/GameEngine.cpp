// GameEngine.cpp
// #include <memory>
#include "GameEngine.h"

#include <algorithm>

// GameEngine Implementation
GameEngine::GameEngine() : window(nullptr), renderer(nullptr), running(false) {}

GameEngine::~GameEngine() { Shutdown(); }

bool GameEngine::Initialize(const char *title, int resx, int resy) {
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
  entityManager = std::make_unique<EntityManager>();

  initialized = true;
  running = true;
  return true;
}

void GameEngine::Run() {
  SDL_Event event;
  Uint32 lastTime = SDL_GetTicks();
  // SDL_Log("Loop Start\n");
  while (running) {
    // Calculate delta time
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime);
    lastTime = currentTime;

    if (!input->IsDisabled())
      input->PreservePrevState();
      
    // Handle events
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT ||
          input->IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
      }
    }
    std::vector<Entity *> &entities = entityManager->getEntityVectorRef();

    // Update input
    if (!input->IsDisabled())
      input->Update();

    // Update game
    Update(deltaTime / 1000.0, entities);

    // Render
    
    if (!renderSystem->IsDisabled())
      Render(entities);

    float delay = std::max(0.0, 1000.0 / 60.0 - deltaTime);
    SDL_Delay(delay);
  }
}

void GameEngine::Update(float deltaTime, std::vector<Entity *> &entities) {
  // Update all entities

  for (auto &entity : entities) {
    if (entity->hasPhysics) {
      physics->ApplyPhysics(entity, deltaTime);
    }

    entity->Update(deltaTime, input.get(), entityManager.get());
    // Apply physics if entity has physics enabled    
  }

  // Process collisions
  collision->ProcessCollisions(entities);
}

void GameEngine::Render(std::vector<Entity *> &entities) {
  if (input->IsKeyPressed(SDL_SCANCODE_0)) {
    renderSystem->SetScalingMode(ScalingMode::CONSTANT_SIZE);
  }
  if (input->IsKeyPressed(SDL_SCANCODE_9)) {
    renderSystem->SetScalingMode(ScalingMode::PROPORTIONAL);
  }

  if (renderSystem->GetScalingMode() == ScalingMode::PROPORTIONAL) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    renderSystem->screenHeight = (float)h;
    renderSystem->screenWidth = (float)w;
  }
  // Clear screen to blue as required
  renderSystem->SetBackgroundColor(0, 100, 200);  // Blue background
  renderSystem->Clear();

  // Render all visible entities
  for (const auto &entity : entities) {
    if (entity->isVisible) {
      renderSystem->RenderEntity(entity);
    }
  }

  renderSystem->Present();
}

// void GameEngine::AddEntity(Entity *entity) { entities.push_back(entity); }

// void GameEngine::RemoveEntity(Entity *entity) {
//   entities.erase(std::remove(entities.begin(), entities.end(), entity),
//                  entities.end());
// }

void GameEngine::Shutdown() {
  entityManager->ClearAllEntities();

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
