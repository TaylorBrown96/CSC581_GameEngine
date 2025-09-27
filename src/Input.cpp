#include "Input.h"

// SDL3: SDL_GetKeyboardState returns const bool*
// keyboardState points to SDL's internal array for the *current* frame.
InputManager::InputManager() {
  
  keyboardState = SDL_GetKeyboardState(&nkeys);
  prevState = (bool*)malloc(sizeof(bool) * nkeys);
  keydiff = (char*)malloc(sizeof(char) * nkeys);
}

void InputManager::PreservePrevState() {
  memcpy(prevState, keyboardState, sizeof(bool) * nkeys);
}

void InputManager::Update() {
  // Fetch the current keyboard state first
  keyboardState = SDL_GetKeyboardState(&nkeys);
}

char* InputManager::GetKeyDiff(int* npkeys) {
  for (int i = 0; i < nkeys; i++) {
    keydiff[i] = (char)keyboardState[i] - (char)prevState[i];
  }
  *npkeys = nkeys;
  return keydiff;
}

bool InputManager::IsKeyPressed(SDL_Scancode scancode) const {
  return keyboardState && keyboardState[scancode];
}

bool InputManager::IsKeyJustPressed(SDL_Scancode scancode) const {
  return keyboardState[scancode] && !prevState[scancode];
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
  return !keyboardState[scancode] && prevState[scancode];
}
