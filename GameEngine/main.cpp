#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>

// Screen dimensions (window width and height)
int SCREEN_W = 1920;
int SCREEN_H = 800;

// Sprite sheet info (sprite sheet is 512x512 frames, 8 frames in one row)
const int FRAME_W = 512;
const int FRAME_H = 512;
const int NUM_FRAMES = 8;

int main(int, char**) {
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    // Create an SDL window that can be resized
    SDL_Window* win = SDL_CreateWindow("Skelly Idle", SCREEN_W, SCREEN_H, SDL_WINDOW_RESIZABLE);
    if (!win) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Create a renderer tied to the window (used for drawing)
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Set renderer background color (white)
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

    // Load sprite sheet bitmap into a surface
    SDL_Surface* surf = SDL_LoadBMP("assets/skelly_idle.bmp");
    if (!surf) {
        std::cerr << "SDL_LoadBMP failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Convert surface to a GPU texture and free the surface
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_DestroySurface(surf);

    if (!tex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Variables for main loop
    bool running = true;
    bool paused = false;
    SDL_Event e;
    int frame = 0;                // current animation frame index
    Uint32 last = SDL_GetTicks(); // timestamp of last frame update
    int frame_ms = 150;           // animation speed (ms per frame)
    float scale = 0.5f;           // scale factor for sprite size (50% default)

    // Print control instructions to console
    std::cout << "Controls:\n"
              << " UP: speed up animation\n"
              << " DOWN: slow down animation\n"
              << " RIGHT: increase size\n"
              << " LEFT: decrease size\n"
              << " SPACE: pause/resume\n"
              << " ESC or window close: quit\n";

    // Main game loop
    while (running) {
        // Handle all pending events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false; // quit when window is closed
            }
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                // Handle key press events
                SDL_Keycode key = e.key.key;
                switch (key) {
                    case SDLK_UP:   // speed up animation
                        frame_ms = std::max(20, frame_ms - 20);
                        std::cout << "FPS: " << (1000 / frame_ms) << "\n";
                        break;
                    case SDLK_DOWN: // slow down animation
                        frame_ms = std::min(1000, frame_ms + 20);
                        std::cout << "FPS: " << (1000 / frame_ms) << "\n";
                        break;
                    case SDLK_RIGHT: // increase scale (up to 150%)
                        scale = std::min(1.5f, scale + 0.1f);
                        std::cout << "Scale: " << scale << "\n";
                        break;
                    case SDLK_LEFT:  // decrease scale (down to 10%)
                        scale = std::max(0.1f, scale - 0.1f);
                        std::cout << "Scale: " << scale << "\n";
                        break;
                    case SDLK_SPACE: // toggle pause
                        paused = !paused;
                        std::cout << (paused ? "Paused\n" : "Resumed\n");
                        break;
                    case SDLK_ESCAPE: // quit
                        running = false;
                        break;
                }
            }
            // Handle window resizing
            if (e.type == SDL_EVENT_WINDOW_RESIZED){
                SCREEN_H = e.window.data2;
                SCREEN_W = e.window.data1;
                printf("Window resized values:\n\tWidth: %d\n\tHeight: %d\n", SCREEN_W, SCREEN_H);
            }
        }

        // Update animation frame (if not paused)
        if (!paused) {
            Uint32 now = SDL_GetTicks();
            if (now - last >= static_cast<Uint32>(frame_ms)) {
                frame = (frame + 1) % NUM_FRAMES; // loop frames
                last = now;
            }
        }

        // Clear screen (set background to white)
        SDL_RenderClear(ren);

        // Source rectangle: current frame from sprite sheet
        SDL_FRect src {
            static_cast<float>(frame * FRAME_W), // x position of frame
            0.0f,                                // y position (row 0)
            static_cast<float>(FRAME_W),         // frame width
            static_cast<float>(FRAME_H)          // frame height
        };

        // Destination rectangle: where to draw the sprite, scaled and centered
        SDL_FRect dst {
            static_cast<float>((SCREEN_W - FRAME_W * scale) / 2.0f),
            static_cast<float>((SCREEN_H - FRAME_H * scale) / 2.0f),
            static_cast<float>(FRAME_W * scale),
            static_cast<float>(FRAME_H * scale)
        };

        // Draw the frame to the screen
        SDL_RenderTexture(ren, tex, &src, &dst);
        SDL_RenderPresent(ren);

        // Delay to limit frame rate (~60fps)
        SDL_Delay(16);
    }

    // Cleanup resources before quitting
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
