#include "GameEngine.h"
// #include <memory>
#include "main.h"

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  GameEngine engine;
  if (!engine.Initialize("Game Engine", 1800, 1000)) {
    return 1;
  }
  engine.GetRenderSystem()->SetScalingMode(ScalingMode::PROPORTIONAL);

  // Create entities
  TestEntity *testEntity = new TestEntity(100, 100);
  testEntity->hasPhysics = true; // Enable physics for TestEntity

  Platform *platform1 = new Platform(300, 800, 300, 75);
  platform1->hasPhysics = false;        // no integration
  platform1->affectedByGravity = false; // no gravity
  platform1->isStatic = true;

  Platform *platform2 = new Platform(800, 650, 300, 75, true);
  platform2->hasPhysics = true; // we want horizontal motion we code ourselves
  platform2->affectedByGravity = false; // but no falling
  platform2->isStatic =
      true; // treat as static for collisions if you have special handling

  // Add entities to the engine
  engine.AddEntity(testEntity);
  engine.AddEntity(platform1);
  engine.AddEntity(platform2);

  SDL_Texture *entityTexture = 
    LoadTexture(engine.GetRenderer(),
                "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
  if (entityTexture) {
    testEntity->SetTexture(entityTexture);
  }

  SDL_Texture *platformTexture =
    LoadTexture(engine.GetRenderer(),
                "media/cartooncrypteque_platform_basicground_idle.bmp");
  if (platformTexture) {
    platform1->SetTexture(platformTexture);
    platform2->SetTexture(platformTexture);
  }

  engine.Run();

  SDL_Log("Cleaning up resources...");
  engine.Shutdown();
  SDL_Log("Shutdown complete. Exiting.");

  return 0;
}