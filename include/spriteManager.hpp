#ifndef SPRITEMAN_H
#define SPRITEMAN_H

#include "renderer.hpp"

class SpriteManager {
    Allocator* spriteMemory;
    Sprite* sprites;
    Renderer* spriteRenderer;
    uint32_t num_sprites;
    public:
    SpriteManager();
    void init();
    void update();
    void assignRenderer(Renderer*);
    Sprite* allocateSpriteMemory();
}

#endif