#include "core/Window.hpp"
#include <iostream>

namespace Engine {

bool Window::create(const std::string& title, int w, int h, Uint32 flags) {
    m_win = SDL_CreateWindow(title.c_str(), w, h, flags);
    if (!m_win) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\\n";
        return false;
    }
    m_width = w; m_height = h;
    return true;
}

void Window::destroy() {
    if (m_win) {
        SDL_DestroyWindow(m_win);
        m_win = nullptr;
    }
}

} // namespace Engine
