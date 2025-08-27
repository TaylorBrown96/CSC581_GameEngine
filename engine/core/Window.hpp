#pragma once
#include <SDL3/SDL.h>
#include <string>

namespace Engine {

class Window {
public:
    bool create(const std::string& title, int w, int h, Uint32 flags = SDL_WINDOW_RESIZABLE);
    void destroy();
    SDL_Window* get() const { return m_win; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    SDL_Window* m_win = nullptr;
    int m_width = 0;
    int m_height = 0;
};

} // namespace Engine
