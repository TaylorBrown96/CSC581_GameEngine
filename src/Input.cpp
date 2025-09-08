#include "Input.h"

// SDL3: SDL_GetKeyboardState returns const bool*
// keyboardState points to SDL's internal array for the *current* frame.
InputManager::InputManager() : keyboardState(nullptr) {}

void InputManager::Update() {
  // Fetch the current keyboard state first
  int numKeys = 0;
  const bool* state = SDL_GetKeyboardState(&numKeys);

  // Snapshot "previous" for any keys we've tracked so far
  // (Don't clear the map; we overwrite values for tracked scancodes.)
  for (auto& kv : previousKeyState) {
    kv.second = state[kv.first];
  }

  // Now make this frame's state available
  keyboardState = state;
}

bool InputManager::IsKeyPressed(SDL_Scancode scancode) const {
  return keyboardState && keyboardState[scancode];
}

bool InputManager::IsKeyJustPressed(SDL_Scancode scancode) const {
  if (!keyboardState) return false;

  const bool current = keyboardState[scancode];
  const auto it = previousKeyState.find(scancode);
  const bool prev = (it != previousKeyState.end()) ? it->second : false;

  // Prepare for next frame: remember what we saw this frame
  const_cast<InputManager*>(this)->previousKeyState[scancode] = current;

  return current && !prev;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
  if (!keyboardState) return false;

  const bool current = keyboardState[scancode];
  const auto it = previousKeyState.find(scancode);
  const bool prev = (it != previousKeyState.end()) ? it->second : false;

  // Prepare for next frame
  const_cast<InputManager*>(this)->previousKeyState[scancode] = current;

  return !current && prev;
}
