#pragma once
#include "render/SpriteSheet.hpp"

namespace Engine {

class SpriteAnimator {
public:
    void setSheet(SpriteSheet* sheet, int totalFrames, int msPerFrame);
    void setPaused(bool p) { m_paused = p; }
    bool paused() const { return m_paused; }

    void addTime(int ms);
    int frame() const { return m_frame; }

    // NEW: runtime control of speed
    void setMsPerFrame(int ms);
    int  msPerFrame() const { return m_msPer; }

private:
    SpriteSheet* m_sheet = nullptr;
    int m_total = 1;
    int m_msPer = 100;
    int m_accum = 0;
    int m_frame = 0;
    bool m_paused = false;
};

} // namespace Engine
