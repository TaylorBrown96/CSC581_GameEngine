#include "GameEngine.h"
#include <memory>

class TestEntity : public Entity {
private:
    int currentFrame;
    Uint32 lastFrameTime;
    int animationDelay;
    int frameCount;
    int frameWidth;
    int frameHeight;
    
public:
    TestEntity(float x, float y) : Entity(x, y, 128, 128) {
        velocityX = 150.0f; // Move right at 150 pixels per second
        currentFrame = 0;
        lastFrameTime = 0;
        animationDelay = 200;
        frameCount = 8;
        frameWidth = 512;
        frameHeight = 512;
    }
    
    void Update(float deltaTime) override {
        // Update animation
        lastFrameTime += (Uint32)(deltaTime * 1000); // Convert to milliseconds
        if (lastFrameTime >= (Uint32)animationDelay) {
            currentFrame = (currentFrame + 1) % frameCount;
            lastFrameTime = 0;
        }
        
        // Bounce off screen edges (demonstrates entity system working)
        if (x <= 0 || x >= 1920 - width) {
            velocityX = -velocityX;
        }
        
        // Reset if falls off bottom (demonstrates physics working)
        if (y > 1080) {
            y = 100;
            velocityY = 0;
        }
    }
    
    void OnCollision(Entity* other) override {
        velocityY = 0.0f;  // Bounce response
    }
    
    // Get current frame for rendering
    bool GetSourceRect(SDL_FRect& out) const override {
        out.x = static_cast<float>(currentFrame * frameWidth);
        out.y = 0.0f;
        out.w = static_cast<float>(frameWidth);
        out.h = static_cast<float>(frameHeight);
        return true;
    }
};


class Platform : public Entity {
public:
    Platform(float x, float y, float w=200, float h=20, bool moving=false) : Entity(x,y,w,h) {
        isStatic = true;
        hasPhysics = false;
        affectedByGravity = false;
        isOneWay = true;              // <- key line
        velocityX = moving ? 100.0f : 0.0f;
        velocityY = 0.0f;
    }

    void Update(float dt) override {
        // Horizontal-only motion for the moving platform
        x += velocityX * dt;
        if (x <= 0 || x >= 1920 - width) velocityX *= -1;
    }

    void LeftRightOccilate(Entity* other) {
        // multiply the xvelocity by -1 to reverse direction
        velocityX *= -1;
    }
};


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