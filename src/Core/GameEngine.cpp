// GameEngine.cpp
// #include <memory>
#include "GameEngine.h"

#include <algorithm>
#include <iostream>
using namespace std;

// GameEngine Implementation
GameEngine::GameEngine() : window(nullptr), renderer(nullptr), running(false), headlessMode(false), jobSystem(2) {}
GameEngine::GameEngine(bool headless) : window(nullptr), renderer(nullptr), running(false), headlessMode(headless), jobSystem(2) {}


GameEngine::~GameEngine() { Shutdown(); }

bool GameEngine::Initialize(const char *title, int resx, int resy, float timeScale = 1.0f) {
  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return false;
  }

  if(timeScale < 0.5 || timeScale > 2.0) {
    SDL_Log("Time scale must be between 0.5 and 2.0");
    return false;
  }

  tickRate = 60.0f * timeScale;
  winsizeX = resx;
  winsizeY = resy;
  if (!headlessMode) {
    // Create window normally
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
  } else {
    // Create hidden window for headless mode
    window = SDL_CreateWindow(title, resx, resy, SDL_WINDOW_HIDDEN);
    if (!window) {
      SDL_Log("Failed to create hidden window: %s", SDL_GetError());
      return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
      SDL_Log("Failed to create renderer: %s", SDL_GetError());
      return false;
    }
  }

  // Initialize systems
  physics = std::make_unique<PhysicsSystem>(3);
  input = std::make_unique<InputManager>();
  renderSystem = std::make_unique<RenderSystem>(renderer, resx, resy);
  rootTimeline = std::make_unique<Timeline>(timeScale, nullptr);
  entityManager = std::make_unique<EntityManager>();
  eventManager = std::make_unique<EventManager>(rootTimeline.get());
  eventManager->RegisterEventHandler(EventType::EVENT_TYPE_COLLISION, new CollisionEventHandler());
  collision = std::make_unique<CollisionSystem>();
  collision->SetEventManager(eventManager.get());

  replayRecorder = std::make_unique<ReplayRecorder>(entityManager.get());
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

    // Handle events {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT ||
          input->IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
      }
    }
    GetRootTimeline()->Update(deltaTime / 1000.0f);
    eventManager->HandleCurrentEvents();
    // } end handle events

    std::vector<Entity *> &entities = entityManager->getEntityVectorRef();

    // Update engine systems in parallel
    UpdateSystemsParallel(deltaTime / 1000.0);
    // update game
    Update(deltaTime / 1000.0, entities);

    replayRecorder->Record();
    replayRecorder->Play();
    

    // Render
    Render(entities);

    float delay = std::max(0.0, 1000.0 / tickRate - deltaTime);
    SDL_Delay(delay);
  }
}

void GameEngine::Update(float deltaTime, std::vector<Entity *> &entities) {
  // Update all entities
  for (auto &entity : entities) {
    float entityDeltaTime = entity->timeline->getDeltaTime();
    entity->Update(entityDeltaTime, input.get(), entityManager.get());

  }
  physics->ApplyPhysicsMultithreaded(entities);
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
    if (entity->rendering.isVisible) {
      renderSystem->RenderEntity(entity);
    }
  }

  renderSystem->Present();
}

void GameEngine::UpdateSystemsParallel(float deltaTime) {
  // Clear the job queue for new frame
  jobSystem.ClearJobs();
  
  // Add parallel system updates
  jobSystem.AddJob([this]() {
    input->Update();
  });
  
  jobSystem.AddJob([this, deltaTime]() {
    rootTimeline->Update(deltaTime);
  });
  
  // Execute all engine system updates in parallel
  jobSystem.ExecuteJobs();
}

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
