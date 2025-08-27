#pragma once
#include <SDL3/SDL.h>
#include <vector>

namespace Engine {

// Lightweight input view passed to the game each frame.
// Uses SDL3’s boolean key state.
struct Input {
    const bool* keys = nullptr;   // from SDL_GetKeyboardState()
    int count = 0;                // number of scancodes
    const std::vector<bool>* prev = nullptr; // previous-frame copy (for edges)

    bool pressed(SDL_Scancode sc) const {
        return keys && static_cast<int>(sc) < count ? keys[sc] : false;
    }
    bool justPressed(SDL_Scancode sc) const {
        if (!prev) return false;
        const bool now = pressed(sc);
        const bool was = static_cast<int>(sc) < static_cast<int>(prev->size()) ? (*prev)[sc] : false;
        return now && !was;
    }
};

} // namespace Engine
