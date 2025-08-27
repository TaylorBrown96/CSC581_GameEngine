#pragma once
#include <SDL3/SDL.h>

namespace Engine {

class Texture {
public:
    bool fromSurface(SDL_Renderer* ren, SDL_Surface* surf);
    bool fromFile(SDL_Renderer* ren, const char* bmpPath);
    void destroy();
    SDL_Texture* get() const { return m_tex; }
    int width() const { return m_w; }
    int height() const { return m_h; }

private:
    SDL_Texture* m_tex = nullptr;
    int m_w = 0, m_h = 0;
};

} // namespace Engine
