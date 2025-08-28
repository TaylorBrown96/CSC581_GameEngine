#include <sprite.hpp>
// create a sprite
int createSprite(Sprite* sprite, float framerate, uint32_t num_frames, float posx, float posy, uint32_t sizex, uint32_t sizey) {
    sprite->visible = 0;
    sprite->sheet = nullptr;
    sprite->source_dest[0] = { 0.0f, 0.0f, sizex, sizey };
    sprite->source_dest[1] = { 0.0f, 0.0f, sizex, sizey };
    sprite->cframerate = framerate;
    sprite->framerate = framerate;
    sprite->current_frame = 0.0f;
    sprite->posx = posx;
    sprite->posy = posy;
    sprite->sizex = sizex;
    sprite->sizey = sizey;
    sprite->scalex = 1.0;
    sprite->scaley = 1.0;
    sprite->num_frames = num_frames;
    return 1;
}

// load a sheet into the sprite
int loadSprite(Sprite* sprite, std::string path) {
    /* Load the texture into sprite->sheet */
    // YOUR CODE HERE  
}

int instantiateSprite(Sprite* src, Sprite* dest) {

}

void animateSprite(Sprite* sprite) {
    sprite->source_dest[0].x = ((uint32_t)sprite->current_frame % sprite->num_frames) * sprite->sizex;
    sprite->source_dest[1].x = sprite->posx;
    sprite->source_dest[1].y = sprite->posy;
    sprite->source_dest[1].w = sprite->scalex;
    sprite->source_dest[1].h = sprite->scaley;
}

void renderSprite(Renderer* renderer, Sprite* sprite) {
    SDL_RenderTexture(renderer, sprite->sheet, &(sprite->source_dest[0]), &(sprite->source_dest[1]));
}

void moveSpriteTo(Sprite* sprite, float posx, float posy) {
    sprite->posx = posx;
    sprite->posy = posy;
}

void scaleSprite(Sprite* sprite, float scalex, float scaley) {
    sprite->scalex = scalex;
    sprite->scaley = scaley;
}
