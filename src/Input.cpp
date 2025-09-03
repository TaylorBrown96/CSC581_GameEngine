#include "Input.h"

InputManager::InputManager() : keyboardState(nullptr) {}

void InputManager::Update() {
  // Store previous key states
  for (auto &pair : previousKeyState) {
    pair.second = keyboardState && keyboardState[pair.first];
  }

  // Get current keyboard state - SDL3 returns const bool* and requires int*
  // parameter
  int numKeys;
  keyboardState = SDL_GetKeyboardState(&numKeys);
}

bool InputManager::IsKeyPressed(SDL_Scancode scancode) const {
  return keyboardState && keyboardState[scancode];
}

bool InputManager::IsKeyJustPressed(SDL_Scancode scancode) const {
  if (!keyboardState)
    return false;

  bool currentState = keyboardState[scancode];
  auto it = previousKeyState.find(scancode);
  bool previousState = (it != previousKeyState.end()) ? it->second : false;

  // Update previous state for next frame (const_cast is needed here)
  const_cast<InputManager *>(this)->previousKeyState[scancode] = currentState;

  return currentState && !previousState;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
  if (!keyboardState)
    return false;

  bool currentState = keyboardState[scancode];
  auto it = previousKeyState.find(scancode);
  bool previousState = (it != previousKeyState.end()) ? it->second : false;

  return !currentState && previousState;
}
