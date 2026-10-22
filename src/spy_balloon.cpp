#include "spy_balloon.h"

void SpyBalloon::onInput() {}

void SpyBalloon::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}