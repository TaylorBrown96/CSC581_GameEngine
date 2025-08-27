#pragma once
#include "render/Texture.hpp"

namespace Engine {

class SpriteSheet {
public:
    void setTexture(Texture* t, int frameW, int frameH, int framesPerRow);
    SDL_FRect frameSrcRect(int frameIndex) const;

private:
    Texture* m_tex = nullptr;
    int m_fw = 0, m_fh = 0, m_fpr = 1;
};

} // namespace Engine
