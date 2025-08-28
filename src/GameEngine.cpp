// GameEngine.cpp
#include "GameEngine.h"
#include <algorithm>

// Static member initialization
int Entity::nextId = 0;

// GameEngine Implementation
GameEngine::GameEngine() : window(nullptr), renderer(nullptr), running(false) {}

GameEngine::~GameEngine() {
    Shutdown();
}

bool GameEngine::Initialize() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    
    // Create window (1920x1080 as required)
    window = SDL_CreateWindow("Game Engine", 1920, 1080, 0);
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
    renderSystem = std::make_unique<RenderSystem>(renderer);
    
    running = true;
    return true;
}

void GameEngine::Run() {
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        
        // Update input
        input->Update();
        
        // Update game
        Update(deltaTime);
        
        // Render
        Render();
    }
}

void GameEngine::Update(float deltaTime) {
    // Update all entities
    for (auto& entity : entities) {
        entity->Update(deltaTime);
        
        // Apply physics if entity has physics enabled
        if (entity->hasPhysics) {
            physics->ApplyPhysics(entity.get(), deltaTime);
        }
    }
    
    // Process collisions
    collision->ProcessCollisions(entities);
}

void GameEngine::Render() {
    // Clear screen to blue as required
    renderSystem->SetBackgroundColor(0, 100, 200); // Blue background
    renderSystem->Clear();
    
    // Render all visible entities
    for (const auto& entity : entities) {
        if (entity->isVisible) {
            renderSystem->RenderEntity(entity.get());
        }
    }
    
    renderSystem->Present();
}

void GameEngine::AddEntity(std::shared_ptr<Entity> entity) {
    entities.push_back(entity);
}

void GameEngine::RemoveEntity(std::shared_ptr<Entity> entity) {
    entities.erase(
        std::remove(entities.begin(), entities.end(), entity),
        entities.end()
    );
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

// Entity Implementation
Entity::Entity(float startX, float startY, float w, float h) 
    : id(nextId++), x(startX), y(startY), width(w), height(h),
      velocityX(0), velocityY(0), texture(nullptr), hasPhysics(true), isVisible(true) {}

Entity::~Entity() {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
}

SDL_FRect Entity::GetBounds() const {
    return {x, y, width, height};
}

void Entity::SetPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Entity::SetTexture(SDL_Texture* tex) {
    texture = tex;
}

// Physics System Implementation
void PhysicsSystem::ApplyPhysics(Entity* entity, float deltaTime) {
    // Apply gravity
    ApplyGravity(entity, deltaTime);
    
    // Update position based on velocity
    entity->x += entity->velocityX * deltaTime;
    entity->y += entity->velocityY * deltaTime;
}

void PhysicsSystem::ApplyGravity(Entity* entity, float deltaTime) {
    entity->velocityY += gravity * deltaTime;
}

// Input Manager Implementation
InputManager::InputManager() : keyboardState(nullptr) {}

void InputManager::Update() {
    // Store previous key states
    for (auto& pair : previousKeyState) {
        pair.second = keyboardState && keyboardState[pair.first];
    }
    
    // Get current keyboard state - SDL3 returns const bool* and requires int* parameter
    int numKeys;
    keyboardState = SDL_GetKeyboardState(&numKeys);
}

bool InputManager::IsKeyPressed(SDL_Scancode scancode) const {
    return keyboardState && keyboardState[scancode];
}

bool InputManager::IsKeyJustPressed(SDL_Scancode scancode) const {
    if (!keyboardState) return false;
    
    bool currentState = keyboardState[scancode];
    auto it = previousKeyState.find(scancode);
    bool previousState = (it != previousKeyState.end()) ? it->second : false;
    
    // Update previous state for next frame (const_cast is needed here)
    const_cast<InputManager*>(this)->previousKeyState[scancode] = currentState;
    
    return currentState && !previousState;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
    if (!keyboardState) return false;
    
    bool currentState = keyboardState[scancode];
    auto it = previousKeyState.find(scancode);
    bool previousState = (it != previousKeyState.end()) ? it->second : false;
    
    return !currentState && previousState;
}

// Collision System Implementation
bool CollisionSystem::CheckCollision(const Entity* entity1, const Entity* entity2) const {
    SDL_FRect rect1 = entity1->GetBounds();
    SDL_FRect rect2 = entity2->GetBounds();
    return CheckCollision(rect1, rect2);
}

bool CollisionSystem::CheckCollision(const SDL_FRect& rect1, const SDL_FRect& rect2) const {
    return SDL_HasRectIntersectionFloat(&rect1, &rect2);
}

void CollisionSystem::ProcessCollisions(std::vector<std::shared_ptr<Entity>>& entities) {
    for (size_t i = 0; i < entities.size(); ++i) {
        for (size_t j = i + 1; j < entities.size(); ++j) {
            if (CheckCollision(entities[i].get(), entities[j].get())) {
                entities[i]->OnCollision(entities[j].get());
                entities[j]->OnCollision(entities[i].get());
            }
        }
    }
}

// Render System Implementation
RenderSystem::RenderSystem(SDL_Renderer* renderer) 
    : renderer(renderer), currentMode(ScalingMode::CONSTANT_SIZE),
      screenWidth(1920), screenHeight(1080), baseWidth(1920), baseHeight(1080) {}

void RenderSystem::SetScalingMode(ScalingMode mode) {
    currentMode = mode;
    SDL_Log("Scaling mode changed to: %s", 
            mode == ScalingMode::CONSTANT_SIZE ? "Constant Size" : "Proportional");
}

void RenderSystem::ToggleScalingMode() {
    currentMode = (currentMode == ScalingMode::CONSTANT_SIZE) ? 
                  ScalingMode::PROPORTIONAL : ScalingMode::CONSTANT_SIZE;
    SDL_Log("Scaling mode toggled to: %s", 
            currentMode == ScalingMode::CONSTANT_SIZE ? "Constant Size" : "Proportional");
}

void RenderSystem::RenderEntity(const Entity* entity) {
    RenderEntity(entity, nullptr);
}

void RenderSystem::RenderEntity(const Entity* entity, const SDL_FRect* sourceRect) {
    if (!entity->texture) return;
    
    SDL_FRect renderRect = CalculateRenderRect(entity);
    SDL_RenderTexture(renderer, entity->texture, sourceRect, &renderRect);
}

SDL_FRect RenderSystem::CalculateRenderRect(const Entity* entity) {
    SDL_FRect rect = {entity->x, entity->y, entity->width, entity->height};
    
    if (currentMode == ScalingMode::PROPORTIONAL) {
        // Scale based on screen size relative to base resolution
        float scaleX = screenWidth / baseWidth;
        float scaleY = screenHeight / baseHeight;
        
        rect.x *= scaleX;
        rect.y *= scaleY;
        rect.w *= scaleX;
        rect.h *= scaleY;
    }
    
    return rect;
}

void RenderSystem::SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void RenderSystem::Clear() {
    SDL_RenderClear(renderer);
}

void RenderSystem::Present() {
    SDL_RenderPresent(renderer);
}