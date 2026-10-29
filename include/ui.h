#ifndef UI_H
#define UI_H

#include <cstdint>
#include <string>

#include "raylib.h"
#include "texture.h"

void drawRectangle(Vector2 center, Vector2 size, Color color);

void drawText(const char* text, Vector2 center, int32_t fontSize, Color textColor);

class TextButton {
public:
    Vector2 position = {0};
    Vector2 size = {0};
    Color backgroundColor = WHITE;
    Color hoverColor = GRAY;
    std::string text;
    Color textColor = BLACK;
    int32_t fontSize = 24;

public:
    TextButton() = default;

    bool leftClicked() const;
    bool isMouseInBounds() const;
    void render() const;
};

class ProgressBar {
public:
    Vector2 position = {0};
    Vector2 size = {0};
    Color barColor = WHITE;
    Color backgroundColor = BLACK;
    Color textColor = RED;
    float borderWidth = 2.0f;
    std::string title;
    int32_t fontSize = 24;

public:
    ProgressBar() = default;

    void render(float currentValue, float maxValue) const;
};

class AbilityStatus {
public:
    Vector2 position = {0};
    Vector2 size = {0};
    Color backgroundColor = BLACK;
    KeyboardKey key;
    const TextureAsset* texture = nullptr;

public:
    AbilityStatus() = default;

    void render(float lastUseTime, float cooldown) const;
};

#endif