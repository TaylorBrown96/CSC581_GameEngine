#include <engine.hpp>

int Engine::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return 0;
    }
    window = nullptr;
    renderer = nullptr;
    if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    spriteManager = new SpriteManager();
    spriteManager.assignRenderer(renderer);
    spriteManager.init();
    /**
     * 
     * Here will be all calls to the entity initialization functions defined in the game
     * 
     * TODO Need to figure out some way to obtain them as function pointers
     * 
     */

}

void Engine::run() {
    while (true) {
        // TODO add overall framerate control here

        clearRenderScreen(renderer);
        
        /**
         * physicsManager.update();
         * audioManager.update();
         * etc..
         */
        spriteManager.update();
        
        presentRenderScreen(renderer);
    }
}