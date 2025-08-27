#include "render/Renderer.hpp"
#include <iostream>

namespace Engine {

bool Renderer::create(SDL_Window* window) {
    m_ren = SDL_CreateRenderer(window, nullptr);
    if (!m_ren) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\\n";
        return false;
    }
    return true;
}

void Renderer::destroy() {
    if (m_ren) {
        SDL_DestroyRenderer(m_ren);
        m_ren = nullptr;
    }
}

void Renderer::clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(m_ren, r, g, b, a);
    SDL_RenderClear(m_ren);
}

void Renderer::present() {
    SDL_RenderPresent(m_ren);
}

} // namespace Engine
