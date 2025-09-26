#pragma once
#include <SDL3/SDL.h>

#include <unordered_map>

class InputManager {
 private:
  bool disabled;
  const bool *keyboardState;  // SDL3 returns const bool*, not const Uint8*
  std::unordered_map<SDL_Scancode, bool> previousKeyState;

 public:
  InputManager();
  void Update();
  void disable() {disabled = true;}
  void enable() {disabled = false;}
  bool IsDisabled() {return disabled;} 
  bool IsKeyPressed(SDL_Scancode scancode) const;
  bool IsKeyJustPressed(SDL_Scancode scancode) const;
  bool IsKeyJustReleased(SDL_Scancode scancode) const;
};
