#include "core/Application.hpp"

int main(int, char**) {
    Engine::Application app;
    Engine::AppConfig cfg;
    cfg.title = "Skelly Idle (Modular)";
    cfg.width = 1920;
    cfg.height = 1080;
    if (!app.init(cfg)) return 1;
    app.run();
    return 0;
}
