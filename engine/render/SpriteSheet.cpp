#include "render/SpriteSheet.hpp"

namespace Engine {

void SpriteSheet::setTexture(Texture* t, int frameW, int frameH, int framesPerRow) {
    m_tex = t; m_fw = frameW; m_fh = frameH; m_fpr = framesPerRow;
}

SDL_FRect SpriteSheet::frameSrcRect(int frameIndex) const {
    int row = frameIndex / m_fpr;
    int col = frameIndex % m_fpr;
    SDL_FRect r { static_cast<float>(col * m_fw), static_cast<float>(row * m_fh),
                  static_cast<float>(m_fw), static_cast<float>(m_fh)};
    return r;
}

} // namespace Engine
