#include "core/Application.hpp"
#include "ExampleGame.hpp"

int main(int, char**) {
    Engine::Application app;
    Engine::AppConfig cfg;
    cfg.title = "CSC581 Game (Engine-driven)";

    ExampleGame game;
    if (!app.init(cfg, &game)) return 1;
    app.run();
    return 0;
}
