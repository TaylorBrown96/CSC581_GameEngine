#ifndef SPRITE_H
#define SPRITE_H

#include <SDL3/SDL.h>

typedef struct Sprite {
    char visible;
    SDL_Texture* sheet;
    SDL_FRect source_dest[2];
    const float cframerate;
    float framerate;
    float current_frame;
    float posx;
    float posy;
    float scalex;
    float scaley;
    uint32_t num_frames;
    uint32_t sizex;
    uint32_t sizey;
} Sprite;

// create a sprite
int createSprite(Sprite* sprite, float framerate, uint32_t num_frames, uint32_t sizex, uint32_t sizey) ;

// load a sheet into the sprite
int loadSprite(Sprite* sprite, std::string path);

int instantiateSprite(Sprite* src, Sprite* dest);

void animateSprite(Sprite* sprite);

void renderSprite(Sprite* sprite);

void moveSpriteTo(Sprite* sprite, float posx, float posy);

void scaleSprite(Sprite* sprite, float scalex, float scaley);

#endif