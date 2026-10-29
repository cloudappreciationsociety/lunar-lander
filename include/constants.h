#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

#include "raylib.h"

namespace constants {

constexpr int32_t SCREEN_WIDTH = 1600;
constexpr int32_t SCREEN_HEIGHT = 1200;
constexpr int32_t TARGET_FPS = 60;
constexpr Vector2 SCREEN_CENTER = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
constexpr Vector2 SCREEN_SIZE = {
    static_cast<float>(SCREEN_WIDTH),
    static_cast<float>(SCREEN_HEIGHT)
};

constexpr float G_ACCEL = 98.0f;

constexpr float FIXED_TIMESTEP = 1.0f / TARGET_FPS;

constexpr static float MAP_LEFT_BOUND = 0.0f;
constexpr static float MAP_RIGHT_BOUND = SCREEN_WIDTH;
constexpr static float MAP_TOP_BOUND = 0.0f;
constexpr static float MAP_BOTTOM_BOUND = SCREEN_HEIGHT;

constexpr static float ROUND_MAX_TIME_SEC = 90.0f;
constexpr static float ROUND_COUNTDOWN_SEC = 3.0f;

constexpr Color BLACK_TINT = {0, 0, 0, 180};

namespace ui {
constexpr int32_t HEADER_FONT_SIZE = 64;
constexpr int32_t FONT_SIZE = 32;
} // namespace ui

} // namespace constants

#endif