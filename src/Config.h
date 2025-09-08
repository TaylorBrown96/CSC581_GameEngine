#pragma once

#include <cstdint>

namespace cfg {

// ------------ Window / Rendering ------------
inline constexpr int    SCREEN_WIDTH          = 1920;
inline constexpr int    SCREEN_HEIGHT         = 1080;
inline constexpr int    TARGET_FPS            = 60;

// Background clear color (SDL uses 0..255)
inline constexpr uint8_t CLEAR_R              = 0;
inline constexpr uint8_t CLEAR_G              = 100;
inline constexpr uint8_t CLEAR_B              = 200;
inline constexpr uint8_t CLEAR_A              = 255;

// ------------ Physics ------------
inline constexpr float GRAVITY_Y              = 1200.0f;   // pixels/s^2 down (+Y)

// ------------ Player / Entities ------------
inline constexpr float PLAYER_SPEED_X         = 350.0f;    // pixels/s
inline constexpr float PLAYER_JUMP_IMPULSE    = -750.0f;   // pixels/s (negative = up)
inline constexpr int   PLAYER_WIDTH           = 128;
inline constexpr int   PLAYER_HEIGHT          = 128;
inline constexpr float DEFAULT_ENTITY_W       = 32.0f;
inline constexpr float DEFAULT_ENTITY_H       = 32.0f;

// ------------ Paths (if you centralize assets) ------------
inline constexpr const char* ASSETS_DIR       = "media/";
} // namespace cfg
