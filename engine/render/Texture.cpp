#include "render/Texture.hpp"
#include <iostream>

namespace Engine {

bool Texture::fromSurface(SDL_Renderer* ren, SDL_Surface* surf) {
    if (m_tex) destroy();
    m_tex = SDL_CreateTextureFromSurface(ren, surf);
    if (!m_tex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\\n";
        return false;
    }
    m_w = surf->w;
    m_h = surf->h;
    return true;
}

bool Texture::fromFile(SDL_Renderer* ren, const char* bmpPath) {
    SDL_Surface* surf = SDL_LoadBMP(bmpPath);
    if (!surf) {
        std::cerr << "SDL_LoadBMP failed: " << SDL_GetError() << "\\n";
        return false;
    }
    bool ok = fromSurface(ren, surf);
    SDL_DestroySurface(surf);
    return ok;
}

void Texture::destroy() {
    if (m_tex) {
        SDL_DestroyTexture(m_tex);
        m_tex = nullptr;
    }
    m_w = m_h = 0;
}

} // namespace Engine
