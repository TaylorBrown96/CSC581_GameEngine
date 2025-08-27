#include "ExampleGame.hpp"
#include "render/Renderer.hpp"
#include "core/Input.hpp"
#include <algorithm>

static constexpr int FRAME_W = 512;
static constexpr int FRAME_H = 512;
static constexpr int NUM_FRAMES = 8;

bool ExampleGame::onInit(Engine::Renderer& renderer) {
    if (!m_assets.loadBMP(m_tex, renderer.get(), "assets/skelly_idle.bmp"))
        return false;

    m_sheet.setTexture(&m_tex, FRAME_W, FRAME_H, NUM_FRAMES);
    m_anim.setSheet(&m_sheet, NUM_FRAMES, 150);
    return true;
}

void ExampleGame::handleControls(const Engine::Input& in, bool& quitFlag) {
    if (in.justPressed(SDL_SCANCODE_ESCAPE)) quitFlag = true;
    if (in.justPressed(SDL_SCANCODE_SPACE))  m_anim.setPaused(!m_anim.paused());

    // Arrows: Up/Down scale, Left/Right speed
    if (in.justPressed(SDL_SCANCODE_UP))    m_scale = std::min(3.0f, m_scale + 0.05f);
    if (in.justPressed(SDL_SCANCODE_DOWN))  m_scale = std::max(0.25f, m_scale - 0.05f);
    if (in.justPressed(SDL_SCANCODE_RIGHT)) m_anim.setMsPerFrame(m_anim.msPerFrame() - 10);
    if (in.justPressed(SDL_SCANCODE_LEFT))  m_anim.setMsPerFrame(m_anim.msPerFrame() + 10);

    // Background colors
    if (in.justPressed(SDL_SCANCODE_1)) m_bg = SDL_Color{  0,  96, 192, 255}; // Blue
    if (in.justPressed(SDL_SCANCODE_2)) m_bg = SDL_Color{ 188,  30,  30, 255}; // Red
    if (in.justPressed(SDL_SCANCODE_3)) m_bg = SDL_Color{ 34, 139,  34, 255}; // Forest green
}

void ExampleGame::onUpdate(double dt, const Engine::Input& input) {
    bool quit = false;
    handleControls(input, quit);
    if (!m_anim.paused()) {
        m_anim.addTime(static_cast<int>(dt * 1000.0));
    }
    // If you want quit-from-game, you can expose a callback to Application; 
    // for now, ESC handling can be moved to Application if needed.
    (void)quit;
}

void ExampleGame::onRender(Engine::Renderer& renderer) {
    int w = 0, h = 0;
    SDL_GetWindowSize(SDL_GetRenderWindow(renderer.get()), &w, &h);

    const int frame = m_anim.frame();
    SDL_FRect src = m_sheet.frameSrcRect(frame);

    SDL_FRect dst {
        (w - src.w * m_scale) * 0.5f,
        (h - src.h * m_scale) * 0.5f,
        src.w * m_scale,
        src.h * m_scale
    };
    SDL_RenderTexture(renderer.get(), m_tex.get(), &src, &dst);
}

void ExampleGame::onShutdown() {
    m_tex.destroy();
}
