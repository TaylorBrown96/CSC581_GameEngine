#pragma once
#include <SDL3/SDL.h>
#include <string>
#include "core/Window.hpp"
#include "render/Renderer.hpp"
#include "render/Texture.hpp"
#include "render/SpriteSheet.hpp"
#include "render/SpriteAnimator.hpp"
#include "utils/AssetManager.hpp"

namespace Engine {

struct AppConfig {
    int width = 1920;
    int height = 1080;
    std::string title = "CSC581 Engine Demo";
};

class Application {
public:
    Application() = default;
    ~Application();

    bool init(const AppConfig& cfg);
    void run();
    void shutdown();

private:
    void handleEvents(bool& running);   // window close
    void handleKeyboard(bool& running); // key controls
    void update(double dt);
    void render();

private:
    AppConfig   m_cfg{};
    Window      m_window;
    Renderer    m_renderer;

    // demo state
    Texture        m_tex;
    SpriteSheet    m_sheet;
    SpriteAnimator m_anim;
    AssetManager   m_assets;

    // control state
    float   m_scale = 0.5f;
    Uint64  m_prevTicks = 0;

    // keyboard state & debouncing
    const bool* m_keys = nullptr;
    int         m_keyCount = 0;

    bool m_prevEsc    = false;
    bool m_prevSpace  = false;
    bool m_prevUp     = false;
    bool m_prevDown   = false;
    bool m_prevLeft   = false;
    bool m_prevRight  = false;
};

} // namespace Engine
