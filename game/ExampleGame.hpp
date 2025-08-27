#pragma once
#include <SDL3/SDL.h>
#include "core/IGame.hpp"
#include "render/Texture.hpp"
#include "render/SpriteSheet.hpp"
#include "render/SpriteAnimator.hpp"
#include "utils/AssetManager.hpp"

class ExampleGame : public Engine::IGame {
public:
    bool onInit(Engine::Renderer& renderer) override;
    void onUpdate(double dt, const Engine::Input& input) override;
    void onRender(Engine::Renderer& renderer) override;
    void onShutdown() override;
    SDL_Color background() const override { return m_bg; }

private:
    // visual
    Engine::Texture      m_tex;
    Engine::SpriteSheet  m_sheet;
    Engine::SpriteAnimator m_anim;
    Engine::AssetManager m_assets;

    // tuning
    float     m_scale = 0.5f;
    SDL_Color m_bg{0,96,192,255}; // engine clears to this

    // helpers
    void handleControls(const Engine::Input& input, bool& quitFlag);
};
