#pragma once
#include <SDL3/SDL.h>

namespace Engine {
class Renderer;
struct Input;

class IGame {
public:
    virtual ~IGame() = default;

    // Called once after the window/renderer exist.
    virtual bool onInit(Renderer& renderer) = 0;

    // Per frame.
    virtual void onUpdate(double dt, const Input& input) = 0;
    virtual void onRender(Renderer& renderer) = 0;

    // Called once before shutdown.
    virtual void onShutdown() = 0;

    // Engine clears to this before onRender().
    virtual SDL_Color background() const = 0;
};
} // namespace Engine
