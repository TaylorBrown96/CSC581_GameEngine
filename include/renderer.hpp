#ifndef REN_H
#define REN_H

#include <SDL3/SDL.h>

typedef SDL_Renderer Renderer;

int clearRenderScreen(Renderer* ren);

int presentRenderScreen(Renderer* ren);
#endif