#include "Input.h"
#include <algorithm>

InputManager::InputManager() : keyboardState(nullptr) {}

void InputManager::Update() {
    sequencesTriggeredThisFrame.clear();

    // Query SDL for current keyboard state
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

        // Just pressed edge?
        if (current && !prev) {
            // Store the press event for sequence checking
            recentPresses.push_back({ (SDL_Scancode)sc, now });
        }

        // Update previousKeyState for next frame
        previousKeyState[(SDL_Scancode)sc] = current;
    }

    keyboardState = state;

    // Trim old press history
    recentPresses.erase(
        std::remove_if(recentPresses.begin(), recentPresses.end(),
            [now, this](const PressRecord& r) {
                return (now - r.timeMs) > maxHistoryMs;
            }),
        recentPresses.end()
    );

    // Sequence Matching
    for (const auto& seq : sequences) {
        const auto& keys = seq.keys;
        if (keys.empty()) continue;

        int seqIndex = (int)keys.size() - 1;
        Uint32 lastMatchTime = now;

        // Scan backwards through press records
        for (int i = (int)recentPresses.size() - 1; i >= 0 && seqIndex >= 0; i--) {
            if (recentPresses[i].key == keys[seqIndex]) {
                Uint32 gap = lastMatchTime - recentPresses[i].timeMs;
                if (gap > seq.maxGapMs)
                    break;

                lastMatchTime = recentPresses[i].timeMs;
                seqIndex--;
            }
        }

        if (seqIndex < 0) {
            sequencesTriggeredThisFrame.push_back(seq.name);
        }
    }
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
    actionToKeys[actionName].push_back(key);
    keyToAction[key] = actionName;
}

void InputManager::AddAction(const std::string& actionName, const std::vector<SDL_Scancode>& keys) {
    for (auto& key : keys) {
        AddAction(actionName, key);
    }
}

bool InputManager::IsActionActive(const std::string& actionName) const {
    if (!keyboardState) return false;

    auto it = actionToKeys.find(actionName);
    if (it == actionToKeys.end()) return false;

    for (auto key : it->second) {
        if (keyboardState[key]) return true;
    }
    return false;
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

// Sequence API
void InputManager::RegisterSequence(const std::string& name,
                                    const std::vector<SDL_Scancode>& keys,
                                    Uint32 maxGapMs) {
    sequences.push_back({ name, keys, maxGapMs });
}

bool InputManager::WasSequenceTriggered(const std::string& name) const {
    return std::find(sequencesTriggeredThisFrame.begin(),
                     sequencesTriggeredThisFrame.end(),
                     name) != sequencesTriggeredThisFrame.end();
}

const std::vector<std::string>& InputManager::GetTriggeredSequences() const {
    return sequencesTriggeredThisFrame;
}
