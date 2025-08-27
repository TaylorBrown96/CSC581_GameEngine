#include "render/SpriteAnimator.hpp"
#include <algorithm>

namespace Engine {

void SpriteAnimator::setSheet(SpriteSheet* sheet, int totalFrames, int msPerFrame) {
    m_sheet = sheet; m_total = totalFrames; m_msPer = msPerFrame; m_accum = 0; m_frame = 0;
}

void SpriteAnimator::addTime(int ms) {
    if (m_paused) return;
    m_accum += ms;
    while (m_accum >= m_msPer) {
        m_accum -= m_msPer;
        m_frame = (m_frame + 1) % m_total;
    }
}

void SpriteAnimator::setMsPerFrame(int ms) {
    // clamp to a sensible range
    m_msPer = std::clamp(ms, 20, 1000);
}

} // namespace Engine
