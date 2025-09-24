#include "GameEngine.h"
#include "Render.h"
#include "Input.h"
#include "NetClient.hpp"

#include <unordered_map>
#include <string>

int main(int argc, char* argv[]) {
    const char* host = "127.0.0.1";
    int port = 7777;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--host" && i + 1 < argc) host = argv[++i];
        else if (a == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
    }

    GameEngine engine;
    if (!engine.Initialize("NetClientDemo", 500, 500)) return 1;

    RenderSystem* render = engine.GetRenderSystem();
    InputManager input;

    SimpleTcpClient net;
    net.connect(host, port);

    const int speed = 5;
    bool running = true;

    while (running) {
        // --- OS/window events (quit / ESC) ---
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.key == SDLK_ESCAPE) running = false;
        }

        // --- Engine input (SDL3 scancodes through InputManager) ---
        input.Update();
        int dx = 0, dy = 0;
        if (input.IsKeyPressed(SDL_SCANCODE_W)) dy -= speed;
        if (input.IsKeyPressed(SDL_SCANCODE_S)) dy += speed;
        if (input.IsKeyPressed(SDL_SCANCODE_A)) dx -= speed;
        if (input.IsKeyPressed(SDL_SCANCODE_D)) dx += speed;
        if (dx || dy) net.move(dx, dy);

        // --- Render with engine's RenderSystem ---
        render->SetBackgroundColor(0, 0, 0, 255);
        render->Clear();

        // Pull a snapshot of current network entities and draw
        auto ents = net.snapshot();
        for (auto& kv : ents) {
            const std::string& id = kv.first;
            const NetEntity&    e  = kv.second;

            // Convert tint 0xRRGGBB
            Uint8 r = (e.tint >> 16) & 0xFF;
            Uint8 g = (e.tint >> 8)  & 0xFF;
            Uint8 b = (e.tint)       & 0xFF;

            SDL_FRect rc{ (float)e.x, (float)e.y, 20.0f, 20.0f };

            // ENV is white + outlined; other players use their tint
            if (id == "ENV") {
                render->FillRect(rc, 255, 255, 255, 255);
                render->StrokeRect(rc, 255, 255, 255, 255);
            } else {
                render->FillRect(rc, r, g, b, 255);
            }
        }

        render->Present();
        SDL_Delay(16);
    }

    net.close();
    return 0;
}
