#pragma once
#include <SDL3/SDL.h>
#include <packetdef.h>
#include <unordered_map>

class InputManager {
 private:
  bool disabled;
  int nkeys;
  bool* prevState;
  const bool *keyboardState;  // SDL3 returns const bool*, not const Uint8*
  char* keydiff;
  /**
   * std::vector<rr_packet> inputQueue;
   */

 public:
  InputManager();
  InputManager(int p_nkeys);
  void PreservePrevState();
  int Size() {return nkeys;}
  void Update();
  char* GetKeyDiff(int* npkeys);
  void disable() {disabled = true;}
  void enable() {disabled = false;}
  bool IsDisabled() {return disabled;} 
  bool IsKeyPressed(SDL_Scancode scancode) const;
  bool IsKeyJustPressed(SDL_Scancode scancode) const;
  bool IsKeyJustReleased(SDL_Scancode scancode) const;
  void SetKeyUnsafe(SDL_Scancode scancode, bool b);
};
