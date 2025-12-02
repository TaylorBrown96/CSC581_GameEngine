#include "Input.h"
#include <algorithm>

InputManager::InputManager() : keyboardState(nullptr) {}

void InputManager::Update() {
    int numKeys = 0;
    const bool* state = SDL_GetKeyboardState(&numKeys);

    Uint32 now = SDL_GetTicks();

    // Detect "just pressed" edges for keys we have in previousKeyState
    for (int sc = 0; sc < numKeys; sc++) {
        bool current = state[sc];
        bool prev = false;

        auto it = previousKeyState.find((SDL_Scancode)sc);
        if (it != previousKeyState.end()) {
            prev = it->second;
        }

        // Update previousKeyState for next frame
        previousKeyState[(SDL_Scancode)sc] = current;
    }

    keyboardState = state;
}

bool InputManager::IsKeyPressed(SDL_Scancode scancode) const {
    return keyboardState && keyboardState[scancode];
}

bool InputManager::IsKeyJustPressed(SDL_Scancode scancode) const {
    if (!keyboardState) return false;

    bool current = keyboardState[scancode];
    bool prev = false;

    auto it = previousKeyState.find(scancode);
    if (it != previousKeyState.end()) prev = it->second;

    const_cast<InputManager*>(this)->previousKeyState[scancode] = current;
    return current && !prev;
}

bool InputManager::IsKeyJustReleased(SDL_Scancode scancode) const {
    if (!keyboardState) return false;

    bool current = keyboardState[scancode];
    bool prev = false;

    auto it = previousKeyState.find(scancode);
    if (it != previousKeyState.end()) prev = it->second;

    const_cast<InputManager*>(this)->previousKeyState[scancode] = current;
    return !current && prev;
}

// Action mapping
void InputManager::AddAction(const std::string& actionName, SDL_Scancode key) {
    ActionTrigger& trigger = actionToKeys[actionName];
    trigger.isKeyChord = false;
    trigger.keys.clear();
    trigger.keys.push_back(key);
    keyToAction[key] = actionName;
}

void InputManager::AddAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys) {
    ActionTrigger& trigger = actionToKeys[actionName];
    trigger.isKeyChord = false;  // Multiple keys as alternatives (OR logic)
    trigger.keys = keys;
    for (auto& key : keys) {
        keyToAction[key] = actionName;
    }
}

void InputManager::AddChordAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys) {
    ActionTrigger& trigger = actionToKeys[actionName];
    trigger.isKeyChord = true;  // All keys must be pressed simultaneously (AND logic)
    trigger.keys = keys;
    // Don't add to keyToAction for chords since individual keys shouldn't trigger the action
}

bool InputManager::IsActionActive(const std::string& actionName) const {
    if (!keyboardState) return false;

    auto it = actionToKeys.find(actionName);
    if (it == actionToKeys.end()) return false;
    
    const ActionTrigger& trigger = it->second;
    
    // If it's a chord, all keys must be pressed (AND logic)
    if (trigger.isKeyChord) {
        for (auto key : trigger.keys) {
            if (!keyboardState[key]) {
                return false;
            }
        }
        return true;
    } else {
        // If it's not a chord, any key can trigger it (OR logic)
        for (auto key : trigger.keys) {
            if (keyboardState[key]) {
                return true;
            }
        }
        return false;
    }
}

std::vector<std::string> InputManager::GetActiveActions() const {
    std::vector<std::string> activeActions;
    if (!keyboardState) return activeActions;

    for (auto& action : actionToKeys) {
        if (IsActionActive(action.first)) {
            activeActions.push_back(action.first);
        }
    }
    return activeActions;
}
