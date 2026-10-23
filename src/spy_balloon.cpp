#include "spy_balloon.h"

#include "constants.h"
#include "texture.h"

SpyBalloon::SpyBalloon() : PhysicsEntity(TextureAsset::get("spy_balloon")) {}

void SpyBalloon::resetMoveDirection() {
    this->moveDirection = {0.0f, 0.0f};
}

void SpyBalloon::onInput(KeyboardKey key) {
    switch (key) {
        case (KeyboardKey::KEY_W): {
            this->moveDirection.y = 1.0f;
            break;
        }

        default: {
            break;
        }
    }

    switch (key) {
        case (KeyboardKey::KEY_A): {
            this->moveDirection.x = -1.0f;
            break;
        }

        case (KeyboardKey::KEY_D): {
            this->moveDirection.x = 1.0f;
            break;
        }

        default: {
            break;
        }
    }
}

void SpyBalloon::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    if (this->moveDirection.y == 1.0f) {
        this->acceleration.y = -constants::G_ACCEL;
    } else {
        this->acceleration.y = constants::G_ACCEL * SpyBalloon::G_ACCEL_MULTIPLIER;
    }

    if (this->moveDirection.x == 1.0f) {
        this->acceleration.x = SpyBalloon::MOVE_ACCEL_X;
    } else if (this->moveDirection.x == -1.0f) {
        this->acceleration.x = -SpyBalloon::MOVE_ACCEL_X;
    } else {
        float dragForce = -SpyBalloon::DRAG * this->velocity.x;

        // If one step of drag would reverse the velocity, set it exactly to zero instead
        if (std::fabs(dragForce * deltaTime) > std::fabs(this->velocity.x)) {
            dragForce = -this->velocity.x / deltaTime;
        }

        this->acceleration.x = dragForce;
    }

    this->collideAndMove(deltaTime);

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void SpyBalloon::render() const {
    PhysicsEntity::render();
}

void SpyBalloon::takeDamage(float damage, Vector2 damageOrigin) {}