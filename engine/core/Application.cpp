#include "core/Application.hpp"
#include <iostream>
#include <algorithm>

namespace Engine {

static const int FRAME_W = 512;
static const int FRAME_H = 512;
static const int NUM_FRAMES = 8;

Application::~Application() { shutdown(); }

bool Application::init(const AppConfig& cfg) {
    m_cfg = cfg;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }
    if (!m_window.create(cfg.title, cfg.width, cfg.height)) return false;
    if (!m_renderer.create(m_window.get())) return false;

    // Load asset and wire up animation
    if (!m_assets.loadBMP(m_tex, m_renderer.get(), "assets/skelly_idle.bmp")) {
        return false;
    }
    m_sheet.setTexture(&m_tex, FRAME_W, FRAME_H, NUM_FRAMES);
    m_anim.setSheet(&m_sheet, NUM_FRAMES, /*msPerFrame*/150);

    // keyboard state pointer
    m_keys = SDL_GetKeyboardState(&m_keyCount);

    m_prevTicks = SDL_GetTicks();
    return true;
}

void Application::handleEvents(bool& running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }
}

void Application::handleKeyboard(bool& running) {
    auto pressed = [&](SDL_Scancode sc) -> bool {
        return (m_keys && static_cast<int>(sc) < m_keyCount) ? m_keys[sc] : false;
    };

    bool esc    = pressed(SDL_SCANCODE_ESCAPE);
    bool space  = pressed(SDL_SCANCODE_SPACE);
    bool up     = pressed(SDL_SCANCODE_UP);
    bool down   = pressed(SDL_SCANCODE_DOWN);
    bool left   = pressed(SDL_SCANCODE_LEFT);
    bool right  = pressed(SDL_SCANCODE_RIGHT);

    // Quit (ESC)
    if (esc && !m_prevEsc) running = false;

    // Pause/Resume (SPACE)
    if (space && !m_prevSpace) m_anim.setPaused(!m_anim.paused());

    // Scale: Up = bigger, Down = smaller
    if (up && !m_prevUp)    m_scale = std::min(3.0f, m_scale + 0.05f);
    if (down && !m_prevDown)m_scale = std::max(0.25f, m_scale - 0.05f);

    // Speed: Right = faster (smaller ms), Left = slower (larger ms)
    if (right && !m_prevRight) m_anim.setMsPerFrame(m_anim.msPerFrame() - 10);
    if (left  && !m_prevLeft)  m_anim.setMsPerFrame(m_anim.msPerFrame() + 10);

    // Update previous states
    m_prevEsc   = esc;
    m_prevSpace = space;
    m_prevUp    = up;
    m_prevDown  = down;
    m_prevLeft  = left;
    m_prevRight = right;
}

void Application::update(double dt) {
    m_anim.addTime(static_cast<int>(dt * 1000.0));
}

void Application::render() {
    // Clear to blue
    m_renderer.clear(0, 96, 192, 255);

    int w = 0, h = 0;
    SDL_GetWindowSize(m_window.get(), &w, &h);

    const int frame = m_anim.frame();
    SDL_FRect src = m_sheet.frameSrcRect(frame);

    SDL_FRect dst {
        (w - src.w * m_scale) * 0.5f,
        (h - src.h * m_scale) * 0.5f,
        src.w * m_scale,
        src.h * m_scale
    };

    SDL_RenderTexture(m_renderer.get(), m_tex.get(), &src, &dst);
    SDL_RenderPresent(m_renderer.get());
}

void Application::run() {
    bool running = true;
    while (running) {
        const Uint64 now = SDL_GetTicks();
        const double dt = (now - m_prevTicks) / 1000.0;
        m_prevTicks = now;

        handleEvents(running);
        handleKeyboard(running);
        if (!running) break;

        update(dt);
        render();

        SDL_Delay(16); // ~60 fps pacing
    }
}

void Application::shutdown() {
    m_tex.destroy();
    m_renderer.destroy();
    m_window.destroy();
    SDL_Quit();
}

} // namespace Engine
