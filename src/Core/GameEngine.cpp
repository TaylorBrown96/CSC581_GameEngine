// GameEngine.cpp
// #include <memory>
#include "GameEngine.h"

#include <algorithm>
#include <iostream>
using namespace std;

// GameEngine Implementation
GameEngine::GameEngine()
    : window(nullptr),
      renderer(nullptr),
      running(false),
      headlessMode(false),
      jobSystem(2) {}

GameEngine::GameEngine(bool headless)
    : window(nullptr),
      renderer(nullptr),
      running(false),
      headlessMode(headless),
      jobSystem(2) {}

GameEngine::~GameEngine() { Shutdown(); }

bool GameEngine::Initialize(const char *title, int resx, int resy, float timeScale /*= 1.0f*/) {
  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return false;
  }

  if (timeScale < 0.5f || timeScale > 2.0f) {
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

  // Replay system initialization: keep the last 5 seconds of history
  replaySystem = std::make_unique<ReplaySystem>(5.0f);
  replaySystem->StartRecording();

  running = true;
  return true;
}

void GameEngine::Run() {
  SDL_Event event;
  Uint32 lastTime = SDL_GetTicks();

  while (running) {
    // Calculate delta time in milliseconds
    Uint32 currentTime = SDL_GetTicks();
    float deltaTimeMs   = static_cast<float>(currentTime - lastTime);
    lastTime            = currentTime;
    float deltaTimeSec  = deltaTimeMs / 1000.0f;

    // Handle events {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT ||
          input->IsKeyPressed(SDL_SCANCODE_ESCAPE)) {
        running = false;
      }

      // Replay toggle: press R to switch between recording and playback
      if (event.type == SDL_EVENT_KEY_DOWN &&
          event.key.scancode == SDL_SCANCODE_R) {
        if (replaySystem) {
          auto mode = replaySystem->GetMode();
          if (mode == ReplaySystem::Mode::Recording) {
            replaySystem->StopRecording();
            replaySystem->StartPlayback();
          } else if (mode == ReplaySystem::Mode::Playing) {
            replaySystem->StopPlayback();
            replaySystem->StartRecording();
          } else { // Idle -> start recording
            replaySystem->StartRecording();
          }
        }
      }
    }
    GetRootTimeline()->Update(deltaTimeSec / 1000.0f);
    eventManager->HandleCurrentEvents();
    // } end handle events

    std::vector<Entity *> &entities = entityManager->getEntityVectorRef();

    // Update engine systems in parallel (input, timeline, etc.)
    UpdateSystemsParallel(deltaTimeSec);

    // Update game simulation + replay
    Update(deltaTimeSec, entities);

    // replayRecorder->Record();
    // replayRecorder->Play();
    

    // Render
    Render(entities);

    float delay = std::max(0.0f, 1000.0f / tickRate - deltaTimeMs);
    SDL_Delay(static_cast<Uint32>(delay));
  }
}

void GameEngine::Update(float deltaTime, std::vector<Entity *> &entities) {
  // If we are in playback mode, only advance the replay and skip live simulation.
  if (replaySystem && replaySystem->GetMode() == ReplaySystem::Mode::Playing) {
    replaySystem->Update(deltaTime);
    return;
  }

  // Normal simulation path (recording or idle)
  for (auto &entity : entities) {
    float entityDeltaTime = entity->timeline->getDeltaTime();
    entity->Update(entityDeltaTime, input.get(), entityManager.get());
  }

  physics->ApplyPhysicsMultithreaded(entities);
  collision->ProcessCollisions(entities);

  // Let the replay system capture this frame at the end of the update.
  if (replaySystem) {
    replaySystem->Update(deltaTime);
  }
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
    renderSystem->screenHeight = static_cast<float>(h);
    renderSystem->screenWidth  = static_cast<float>(w);
  }

   // Background: blue normally, forest green during replay playback
  if (replaySystem && replaySystem->GetMode() == ReplaySystem::Mode::Playing) {
    // Forest green
    renderSystem->SetBackgroundColor(34, 139, 34);
  } else {
    // Normal blue
    renderSystem->SetBackgroundColor(0, 100, 200);
  }
  renderSystem->Clear();

  // Render all visible entities
  for (const auto& entity : entities) {
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
  if (entityManager) {
    entityManager->ClearAllEntities();
  }

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
