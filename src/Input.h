#pragma once
#include <SDL3/SDL.h>
#include <unordered_map>

class InputManager {
private:
  const bool *keyboardState; // SDL3 returns const bool*, not const Uint8*
  std::unordered_map<SDL_Scancode, bool> previousKeyState;

public:
  InputManager();
  void Update();

  bool IsKeyPressed(SDL_Scancode scancode) const;
  bool IsKeyJustPressed(SDL_Scancode scancode) const;
  bool IsKeyJustReleased(SDL_Scancode scancode) const;
};
