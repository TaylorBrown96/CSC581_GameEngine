#pragma once
#include <SDL3/SDL.h>

#include "Entity.h"

enum class ScalingMode {
  CONSTANT_SIZE,  // Pixel-based
  PROPORTIONAL    // Percentage-based
};

class RenderSystem {
 private:
  bool disabled;
  SDL_Renderer *renderer;
  ScalingMode currentMode;

  float baseWidth, baseHeight;  // Reference resolution for proportional scaling

 public:
  void disable() {disabled = true;}
  void enable() {disabled = false;}
  bool IsDisabled() {return disabled;} 
  float screenWidth, screenHeight;

  explicit RenderSystem(SDL_Renderer *renderer);

  RenderSystem(SDL_Renderer *renderer, int width, int height);

  void SetScalingMode(ScalingMode mode);
  ScalingMode GetScalingMode() const { return currentMode; }
  void ToggleScalingMode();

  // Auto: asks the entity for a source rect (frame) via GetSourceRect(...)
  void RenderEntity(const Entity *entity);

  // Manual: render with an explicit source rect (or nullptr for full texture)
  void RenderEntity(const Entity *entity, const SDL_FRect *sourceRect);

  void SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
  void Clear();
  void Present();

 private:
  SDL_FRect CalculateRenderRect(const Entity *entity);
};

SDL_Texture *LoadTexture(SDL_Renderer *renderer, const char *path);
SDL_Texture *CreateColoredTexture(SDL_Renderer *renderer, Uint8 width, Uint8 height, Uint8 r, Uint8 g, Uint8 b);
