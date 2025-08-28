#include <spriteManager.hpp>

SpriteManager::SpriteManager() {
    spriteMemory = nullptr;
    sprites = nullptr;
    spriteRenderer = nullptr;
    num_sprites = 0;
}
void SpriteManager::assignRenderer(Renderer* ren) {
    spriteRenderer = ren;
}

void SpriteManager::init() {
    spriteMemory = createAllocator(2048 * sizeof(char));
    sprites = spriteMemory.base;
}

void SpriteManager::update() {
    for (uint32_t n = 0; n < num_sprites; n++) {
        if (sprites[n].framerate > 0.0)
            animateSprite(&sprites[n]);
        if (sprites[n].visible)
            renderSprite(spriteRenderer, &sprites[n]);
    }
}

Sprite* SpriteManager::allocateSpriteMemory() {
    Sprite* top = (Sprite*)push(spriteMemory, sizeof(Sprite));
    memset(top, 0, sizeof(Sprite));
    num_sprites++;
    return top;
}


