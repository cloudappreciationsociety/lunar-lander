#include "ui.h"

#include <cmath>

#include "constants.h"
#include "raymath.h"

void drawRectangle(Vector2 center, Vector2 size, Color color) {
    DrawRectangle(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, color);
}

void drawText(const char* text, Vector2 center, int32_t fontSize, Color textColor) {
    // The spacing used in DrawText is apparently fontSize / 10: https://github.com/raysan5/raylib/blob/master/src/rtext.c#L1211
    Vector2 textBounds =
        MeasureTextEx(GetFontDefault(), text, fontSize, static_cast<float>(fontSize) / 10);

    DrawText(text, center.x - textBounds.x / 2, center.y - textBounds.y / 2, fontSize, textColor);
}

void drawTextLeftAligned(
    const char* text,
    Vector2 topLeftCorner,
    int32_t fontSize,
    Color textColor
) {
    DrawText(text, topLeftCorner.x, topLeftCorner.y, fontSize, textColor);
}

bool TextButton::leftClicked() const {
    return this->isMouseInBounds() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool TextButton::isMouseInBounds() const {
    Vector2 mousePosition = GetMousePosition();

    return (
        std::fabs(mousePosition.x - this->position.x) < this->size.x / 2
        && std::fabs(mousePosition.y - this->position.y) < this->size.y / 2
    );
}

void TextButton::render() const {
    drawRectangle(
        this->position,
        this->size,
        this->isMouseInBounds() ? this->hoverColor : this->backgroundColor
    );
    drawText(this->text.c_str(), this->position, this->fontSize, this->textColor);
}

void ProgressBar::render(float currentValue, float maxValue) const {
    float progressRatio = currentValue / maxValue;

    // The background rectangle
    drawRectangle(this->position, this->size, this->backgroundColor);

    float innerWidth = this->size.x - 2 * this->borderWidth;
    float innerHeight = this->size.y - 2 * this->borderWidth;
    float barWidth = innerWidth * progressRatio;
    float innerLeftEdge = this->position.x - innerWidth / 2.0f;

    // The progress bar
    drawRectangle(
        {innerLeftEdge + barWidth / 2.0f, this->position.y},
        {barWidth, innerHeight},
        this->barColor
    );

    // The text that's left-aligned in the bar and vertically centered
    drawTextLeftAligned(
        TextFormat("%s: %i", this->title.c_str(), static_cast<int32_t>(std::ceil(currentValue))),
        {this->position.x - this->size.x / 2.0f + 2 * this->borderWidth + (this->fontSize / 5.0f),
         this->position.y - this->fontSize / 2.0f},
        this->fontSize,
        this->textColor
    );
}

void AbilityStatus::render(float lastUseTime, float cooldown) const {
    drawRectangle(this->position, this->size, this->backgroundColor);

    float now = static_cast<float>(GetTime());
    float availableAt = lastUseTime + cooldown;

    Texture2D tex = this->texture->getTexture2D();
    DrawTexturePro(
        tex,
        {0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height)},
        {this->position.x, this->position.y, this->size.x / 2.0f, this->size.y / 2.0f},
        {this->size.x / 4.0f, this->size.y / 4.0f},
        0.0f,
        WHITE
    );

    int32_t fontSize = constants::ui::FONT_SIZE;

    // Tint with cooldown if we're not ready
    if (now < availableAt) {
        drawRectangle(this->position, this->size, constants::BLACK_TINT);
        drawText(
            TextFormat("%i", static_cast<int32_t>(std::ceil(availableAt - now))),
            this->position,
            fontSize,
            WHITE
        );
    }

    // The activation key
    drawTextLeftAligned(
        GetKeyName(this->key),
        {this->position.x - this->size.x / 2.0f + 5.0f, this->position.y - this->size.y / 2.0f},
        fontSize,
        WHITE
    );
}