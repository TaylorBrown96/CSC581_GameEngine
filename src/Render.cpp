#include "Render.h"
#include <SDL3/SDL.h>

RenderSystem::RenderSystem(SDL_Renderer* renderer)
    : renderer(renderer),
      currentMode(ScalingMode::CONSTANT_SIZE),
      screenWidth(1920.0f),
      screenHeight(1080.0f),
      baseWidth(1920.0f),
      baseHeight(1080.0f) {}

void RenderSystem::SetScalingMode(ScalingMode mode) {
    currentMode = mode;
    SDL_Log("Scaling mode changed to: %s",
            mode == ScalingMode::CONSTANT_SIZE ? "Constant Size" : "Proportional");
}

void RenderSystem::ToggleScalingMode() {
    currentMode = (currentMode == ScalingMode::CONSTANT_SIZE)
                    ? ScalingMode::PROPORTIONAL
                    : ScalingMode::CONSTANT_SIZE;
    SDL_Log("Scaling mode toggled to: %s",
            currentMode == ScalingMode::CONSTANT_SIZE ? "Constant Size" : "Proportional");
}

void RenderSystem::RenderEntity(const Entity* entity) {
    if (!entity) return;
    // Use your entity's texture member directly to match your current codebase.
    // If you have an accessor, replace with: SDL_Texture* tex = entity->GetTexture();
    SDL_Texture* tex = entity->texture;
    if (!tex) return;

    SDL_FRect dst = CalculateRenderRect(entity);

    SDL_FRect src;
    const SDL_FRect* psrc = nullptr;
    if (entity->GetSourceRect(src)) {
        psrc = &src; // draw the current frame
    }

    SDL_RenderTexture(renderer, tex, psrc, &dst);
}

void RenderSystem::RenderEntity(const Entity* entity, const SDL_FRect* sourceRect) {
    if (!entity) return;
    SDL_Texture* tex = entity->texture;
    if (!tex) return;

    SDL_FRect dst = CalculateRenderRect(entity);
    SDL_RenderTexture(renderer, tex, sourceRect, &dst);
}

SDL_FRect RenderSystem::CalculateRenderRect(const Entity* entity) {
    SDL_FRect rect{ entity->x, entity->y, entity->width, entity->height };

    if (currentMode == ScalingMode::PROPORTIONAL) {
        const float scaleX = screenWidth  / baseWidth;
        const float scaleY = screenHeight / baseHeight;

        rect.x *= scaleX;
        rect.y *= scaleY;
        rect.w *= scaleX;
        rect.h *= scaleY;
    }

    return rect;
}

void RenderSystem::SetBackgroundColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void RenderSystem::Clear() {
    SDL_RenderClear(renderer);
}

void RenderSystem::Present() {
    SDL_RenderPresent(renderer);
}


SDL_Texture* LoadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    
    if (!texture) {
        return nullptr;
    }

    return texture;
}