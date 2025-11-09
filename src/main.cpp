#include "Core/GameEngine.h"
// #include <memory>
#include "main.h"

// Simple state structs for replay tracks
struct PlayerReplayState {
  float x;
  float y;
  float vx;
  float vy;
  int   currentFrame;
  int   currentTextureState;
};

struct PlatformReplayState {
  float x;
  float y;
};

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;

  GameEngine engine;
  if (!engine.Initialize("Game Engine", 1800, 1000, 1.0f)) {
    return 1;
  }
  engine.GetRenderSystem()->SetScalingMode(ScalingMode::PROPORTIONAL);

  Timeline *halfTimeline   = new Timeline(0.5f, engine.GetRootTimeline());
  Timeline *doubleTimeline = new Timeline(2.0f, engine.GetRootTimeline());
  (void)doubleTimeline; // unused for now
  
  // Create entities
  TestEntity *testEntity =
      new TestEntity(100, 100, engine.GetRootTimeline(), engine.GetRenderer());
  // TestEntity already enables physics in its constructor

  Platform *platform1 =
      new Platform(300, 800, 300, 75, false, halfTimeline, engine.GetRenderer());
  // platform1 has collision enabled but no physics (static platform)

  Platform *platform2 =
      new Platform(800, 650, 300, 75, true, engine.GetRootTimeline(), engine.GetRenderer());
  // platform2 has collision and physics enabled (moving platform)

  // Add entities to the engine
  engine.GetEntityManager()->AddEntity(testEntity);
  engine.GetEntityManager()->AddEntity(platform1);
  engine.GetEntityManager()->AddEntity(platform2);

  // Load textures for entities
  SDL_Texture *entityTexture = LoadTexture(
      engine.GetRenderer(),
      "media/cartooncrypteque_character_skellywithahat_idleright.bmp");
  if (entityTexture) {
    Texture tex;
    tex.sheet = entityTexture;
    tex.num_frames_x = 8;
    tex.num_frames_y = 0;
    tex.frame_width = 512;
    tex.frame_height = 512;
    tex.loop = true;
    testEntity->SetTexture(0, &tex);
  }

  SDL_Texture *platformTexture =
      LoadTexture(engine.GetRenderer(),
                  "media/cartooncrypteque_platform_basicground_idle.bmp");
  if (platformTexture) {
    Texture tex;
    tex.sheet = platformTexture;
    tex.num_frames_x = 1;
    tex.num_frames_y = 1;
    tex.frame_width = 200;
    tex.frame_height = 20;
    tex.loop = true;
    platform1->SetTexture(0, &tex);
    platform2->SetTexture(0, &tex);
  }
  // -------------------------------------------------------
  // Replay system hook-up (modular tracks)
  // -------------------------------------------------------
  if (auto *replay = engine.GetReplaySystem()) {
    // Track 1: Player / TestEntity
    replay->RegisterTrack<PlayerReplayState>(
      "player",
      // Capture current player state
      [testEntity]() -> PlayerReplayState {
        PlayerReplayState s{};
        SDL_FRect bounds = testEntity->GetBounds();
        s.x  = bounds.x;
        s.y  = bounds.y;
        s.vx = testEntity->GetVelocityX();
        s.vy = testEntity->GetVelocityY();

        // Capture animation state
        s.currentFrame        = testEntity->rendering.currentFrame;
        s.currentTextureState = testEntity->rendering.currentTextureState;

        return s;
      },
      // Apply recorded player state
      [testEntity](const PlayerReplayState &s) {
        testEntity->SetPosition(s.x, s.y);
        testEntity->SetVelocity(s.vx, s.vy);

        // Restore animation state
        testEntity->rendering.currentFrame        = s.currentFrame;
        testEntity->rendering.currentTextureState = s.currentTextureState;
      }
    );

    // Track 2: Moving platform (platform2) so the world matches the replay
    replay->RegisterTrack<PlatformReplayState>(
      "moving_platform",
      [platform2]() -> PlatformReplayState {
        PlatformReplayState s{};
        SDL_FRect bounds = platform2->GetBounds();
        s.x = bounds.x;
        s.y = bounds.y;
        return s;
      },
      [platform2](const PlatformReplayState &s) {
        platform2->SetPosition(s.x, s.y);
      }
    );
  }

  engine.Run();

  SDL_Log("Cleaning up resources...");
  engine.Shutdown();
  SDL_Log("Shutdown complete. Exiting.");

  return 0;
}
