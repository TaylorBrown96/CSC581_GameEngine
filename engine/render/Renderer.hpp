#pragma once
#include <SDL3/SDL.h>

namespace Engine {

class Renderer {
public:
    bool create(SDL_Window* window);
    void destroy();

    SDL_Renderer* get() const { return m_ren; }
    void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);
    void present();

private:
    SDL_Renderer* m_ren = nullptr;
};

} // namespace Engine
