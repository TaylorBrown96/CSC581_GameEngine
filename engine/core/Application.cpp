#include "core/Application.hpp"
#include <iostream>
#include <algorithm>

namespace Engine {

Application::~Application() { shutdown(); }

bool Application::init(const AppConfig& cfg, IGame* game) {
    m_cfg = cfg;
    m_game = game;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    if (!m_window.create(cfg.title, cfg.width, cfg.height)) return false;
    if (!m_renderer.create(m_window.get())) return false;

    // Input pointers
    m_keys = SDL_GetKeyboardState(&m_keyCount);
    m_prevKeys.assign(m_keyCount, false);

    if (!m_game || !m_game->onInit(m_renderer)) {
        std::cerr << "Game onInit() failed.\n";
        return false;
    }

    m_prevTicks = SDL_GetTicks();
    return true;
}

void Application::pumpEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) running = false;
    }
}

void Application::renderFrame() {
    SDL_Color bg = m_game ? m_game->background() : SDL_Color{0,96,192,255};
    m_renderer.clear(bg.r, bg.g, bg.b, bg.a);
    if (m_game) m_game->onRender(m_renderer);
    SDL_RenderPresent(m_renderer.get());
}

void Application::run() {
    bool running = true;
    while (running) {
        const Uint64 now = SDL_GetTicks();
        const double dt = (now - m_prevTicks) / 1000.0;
        m_prevTicks = now;

        pumpEvents(running);

        // Build input view and update game
        Input input{ m_keys, m_keyCount, &m_prevKeys };
        if (m_game) m_game->onUpdate(dt, input);

        renderFrame();

        // capture prev key states
        if (m_keyCount > 0) {
            if ((int)m_prevKeys.size() != m_keyCount) m_prevKeys.assign(m_keyCount, false);
            for (int i=0; i<m_keyCount; ++i) m_prevKeys[i] = m_keys[i];
        }

        SDL_Delay(1); // light yield
    }
}

void Application::shutdown() {
    if (m_game) {
        m_game->onShutdown();
        m_game = nullptr;
    }
    m_renderer.destroy();
    m_window.destroy();
    SDL_Quit();
}

} // namespace Engine
