#include <renderer.hpp>

int clearRenderScreen(Renderer* ren) {
    SDL_RenderClear(renderer);
}

int presentRenderScreen(Renderer* ren) {
    SDL_RenderPresent(renderer);
}