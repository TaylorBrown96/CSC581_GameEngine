#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include "core/Window.hpp"
#include "core/IGame.hpp"
#include "core/Input.hpp"
#include "render/Renderer.hpp"

namespace Engine {

struct AppConfig {
    int width = 1920;
    int height = 1080;
    std::string title = "CSC581 Engine";
};

class Application {
public:
    Application() = default;
    ~Application();

    bool init(const AppConfig& cfg, IGame* game); // game provided here
    void run();
    void shutdown();

private:
    void pumpEvents(bool& running);
    void renderFrame();

private:
    AppConfig   m_cfg{};
    Window      m_window;
    Renderer    m_renderer;

    IGame*      m_game = nullptr;

    // timing
    Uint64  m_prevTicks = 0;

    // input
    const bool* m_keys = nullptr;
    int         m_keyCount = 0;
    std::vector<bool> m_prevKeys; // copied after each frame
};

} // namespace Engine
