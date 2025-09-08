#include "GameEngine.h"
#include "Config.h"
#include "main.h"
#include <string>

// Provided by the engine
extern SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* path);

int main(int argc, char* argv[]) {
  (void)argc; (void)argv;

  GameEngine engine;
  if (!engine.Initialize("Individual Game", cfg::SCREEN_WIDTH, cfg::SCREEN_HEIGHT)) return 1;
  if (auto* rs = engine.GetRenderSystem()) rs->SetScalingMode(ScalingMode::CONSTANT_SIZE);

  // -----------------------------
  // Entities
  // -----------------------------
  auto* player = new Player(100.f, 100.f);

  // Static platform: mossy stone
  auto* floor = new Ground(100.f, 820.f, 600.f, 90.f);

  // Moving platform: bridge (oscillate 220px at 100 px/s)
  auto* ledge = new MovingGround(820.f, 660.f, 320.f, 40.f, 220.f, 100.f);

  // Enemy path (swirling orb)
  std::vector<SDL_FPoint> loopPath = {
    {1200.f, 300.f}, {1450.f, 300.f}, {1450.f, 520.f}, {1200.f, 520.f}
  };
  auto* bot = new PatrolBot(loopPath, 140.f);

  // -----------------------------
  // Textures (darkworld set)
  // -----------------------------
  const std::string base = std::string(cfg::ASSETS_DIR);

  // Player sheets
  SDL_Texture* tIdle = LoadTexture(engine.GetRenderer(), (base + "darkworld_character_cainhurst_idle.bmp").c_str());
  SDL_Texture* tLeft = LoadTexture(engine.GetRenderer(), (base + "darkworld_character_cainhurst_left.bmp").c_str());   // 4 frames
  SDL_Texture* tRight= LoadTexture(engine.GetRenderer(), (base + "darkworld_character_cainhurst_right.bmp").c_str());  // 4 frames
  SDL_Texture* tJump = LoadTexture(engine.GetRenderer(), (base + "darkworld_character_cainhurst_jump.bmp").c_str());   // 1 frame
  SDL_Texture* tFall = LoadTexture(engine.GetRenderer(), (base + "darkworld_character_cainhurst_fall.bmp").c_str());   // 1 frame
  player->SetSheets(tIdle, tLeft, tRight, tJump, tFall);

  // Platforms (static images)
  SDL_Texture* tMossy   = LoadTexture(engine.GetRenderer(), (base + "darkworld_platform_mossystone.bmp").c_str());
  SDL_Texture* tBridge  = LoadTexture(engine.GetRenderer(), (base + "darkworld_platform_lanternbridge.bmp").c_str());
  if (tMossy)  floor->SetTexture(tMossy);
  if (tBridge) ledge->SetTexture(tBridge);

  // Enemy (swirling orb: 4 frames)
  SDL_Texture* tOrb = LoadTexture(engine.GetRenderer(), (base + "darkworld_enemy_swirlingorb.bmp").c_str());
  if (tOrb) bot->SetTextureAndLayout(tOrb);  // sets 4x1 grid internally

  // -----------------------------
  // Register & Run
  // -----------------------------
  engine.AddEntity(floor);
  engine.AddEntity(ledge);
  engine.AddEntity(bot);
  engine.AddEntity(player);

  engine.Run();
  engine.Shutdown();
  return 0;
}
