#include "GameEngine.h"
#include <memory>
#include "main.h"

int main(int argc, char* argv[]) {
    
    GameEngine engine;
    if (!engine.Initialize()) {
        return 1;
    }
    
    // Create entities
    auto testEntity = std::make_shared<TestEntity>(100, 100);
    testEntity->hasPhysics = true; // Enable physics for TestEntity

    auto platform1 = std::make_shared<Platform>(400, 800, 300, 50);
    platform1->hasPhysics = false;         // no integration
    platform1->affectedByGravity = false;  // no gravity
    platform1->isStatic = true;

    auto platform2 = std::make_shared<Platform>(800, 600, 300, 50, true);
    platform2->hasPhysics = true;          // we want horizontal motion we code ourselves
    platform2->affectedByGravity = false;  // but no falling
    platform2->isStatic = true;            // treat as static for collisions if you have special handling
    
    // Add entities to the engine
    engine.AddEntity(testEntity);
    engine.AddEntity(platform1);
    engine.AddEntity(platform2);

    SDL_Texture* entityTexture = LoadTexture(engine.GetRenderer(), "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
    if (entityTexture) {
        testEntity->SetTexture(entityTexture);
    }

    SDL_Texture* platformTexture = LoadTexture(engine.GetRenderer(), "media/cartooncrypteque_platform_basicground_idle.bmp");
    if (platformTexture) {
        platform1->SetTexture(platformTexture);
        platform2->SetTexture(platformTexture);
    }

    SDL_Event event; // For event handling
    Uint32 lastTime = SDL_GetTicks(); // For frame timing
    Uint32 lastInputCheck = 0; // For input checking
    const Uint32 INPUT_CHECK_DELAY = 100; // Check input every 100ms to avoid spam

    Uint32 variableTime = 0; // For variable timing

    // inside while(true) in main.cpp
    while (true) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // ---- Event Handling ----
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) goto cleanup;
        }

        engine.GetInput()->Update();

        // Exit key
        if (engine.GetInput()->IsKeyPressed(SDL_SCANCODE_ESCAPE)) break;

        // ---- Player Movement ----

        /*Left & Right Movement*/
        float newVX = 0.0f;
        if (engine.GetInput()->IsKeyPressed(SDL_SCANCODE_A)||engine.GetInput()->IsKeyPressed(SDL_SCANCODE_LEFT)) newVX -= 200.0f; // move left
        if (engine.GetInput()->IsKeyPressed(SDL_SCANCODE_D)||engine.GetInput()->IsKeyPressed(SDL_SCANCODE_RIGHT)) newVX += 200.0f; // move right
        testEntity->velocityX = newVX;

        /*Jump*/
       if (engine.GetInput()->IsKeyPressed(SDL_SCANCODE_SPACE) && testEntity->grounded) {
            testEntity->velocityY = -450.0f;
            testEntity->grounded = false;
        }

        // ---- Update World ----
        engine.Update(deltaTime);
        engine.GetCollision()->ProcessCollisions(engine.GetEntities());

        // ---- Render Frame ----
        engine.Render();

        // ---- Frame Timing ----
        SDL_Delay(16); // ~60 FPS cap
    }

    cleanup:
    SDL_Log("Cleaning up resources...");

    return 0;
}