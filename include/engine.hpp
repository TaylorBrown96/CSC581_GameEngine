#ifndef ENGINE_H
#define ENGINE_H

#include <spriteManager.hpp>
#include <renderer.hpp>
#include <window.hpp>

class Engine {
    SpriteManager* spriteManager;
    /**
     * Other Managers will go here
     * PhysicsManager* physicsManager
     * AudioManager* audioManager
     */
    
    Window* window;
    Renderer* renderer;
    std::string title;
    uint32_t width;
    uint32_t height;
public:
    
    init();
    run();
}

#endif