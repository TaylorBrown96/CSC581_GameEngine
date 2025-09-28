#include "Input.h"
#include <algorithm>

// SDL3: SDL_GetKeyboardState returns const bool*
// keyboardState points to SDL's internal array for the *current* frame.
InputManager::InputManager() : keyboardState(nullptr) {
}

void InputManager::Update() {
  // Fetch the current keyboard state first
  int numKeys = 0;
  const bool *state = SDL_GetKeyboardState(&numKeys);

  // Snapshot "previous" for any keys we've tracked so far
  // (Don't clear the map; we overwrite values for tracked scancodes.)
  for (auto &kv : previousKeyState) {
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
  const_cast<InputManager *>(this)->previousKeyState[scancode] = current;

  return current && !prev;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
  if (!keyboardState) return false;

  const bool current = keyboardState[scancode];
  const auto it = previousKeyState.find(scancode);
  const bool prev = (it != previousKeyState.end()) ? it->second : false;

  // Prepare for next frame
  const_cast<InputManager *>(this)->previousKeyState[scancode] = current;

  return !current && prev;
}

// Action mapping methods
void InputManager::AddAction(const std::string& actionName, SDL_Scancode key) {
    // Add to action-to-keys mapping
    actionToKeys[actionName].push_back(key);
    
    // Add to key-to-action mapping
    keyToAction[key] = actionName;
}

void InputManager::AddAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys) {
    // Add all keys for this action
    for (const auto& key : keys) {
        AddAction(actionName, key);
    }
}

bool InputManager::IsActionActive(const std::string& actionName) const {
    if (!keyboardState) return false;
    
    auto it = actionToKeys.find(actionName);
    if (it == actionToKeys.end()) return false;
    
    // Check if any of the keys for this action are pressed
    for (const auto& key : it->second) {
        if (keyboardState[key]) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> InputManager::GetActiveActions() const {
    std::vector<std::string> activeActions;
    
    if (!keyboardState) return activeActions;
    
    for (const auto& actionPair : actionToKeys) {
        const std::string& actionName = actionPair.first;
        if (IsActionActive(actionName)) {
            activeActions.push_back(actionName);
        }
    }
    
    return activeActions;
}
