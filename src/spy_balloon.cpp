#include "spy_balloon.h"

#include "constants.h"
#include "texture.h"

SpyBalloon::SpyBalloon() : Actor(TextureAsset::get("spy_balloon")) {
    this->health = SpyBalloon::STARTING_HEALTH;
}

void SpyBalloon::clampPositionInMap() {
    const float halfWidth = this->colliderDimensions.x / 2.0f;
    const float halfHeight = this->colliderDimensions.y / 2.0f;

    const float minX = constants::MAP_LEFT_BOUND + halfWidth;
    const float maxX = constants::MAP_RIGHT_BOUND - halfWidth;
    const float minY = constants::MAP_TOP_BOUND + halfHeight;

    if (this->position.x < minX) {
        this->position.x = minX;
        if (this->velocity.x < 0.0f) {
            this->velocity.x = 0.0f;
        }
    } else if (this->position.x > maxX) {
        this->position.x = maxX;
        if (this->velocity.x > 0.0f) {
            this->velocity.x = 0.0f;
        }
    }

    if (this->position.y < minY) {
        this->position.y = minY;
        if (this->velocity.y < 0.0f) {
            this->velocity.y = 0.0f;
        }
    }
}

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
        this->addFuel(SpyBalloon::MOVE_FUEL_DRAIN);
    } else if (this->moveDirection.x == -1.0f) {
        this->acceleration.x = -SpyBalloon::MOVE_ACCEL_X;
        this->addFuel(SpyBalloon::MOVE_FUEL_DRAIN);
    } else {
        this->applyDragX(deltaTime, SpyBalloon::DRAG);
    }

    this->collideAndMove(deltaTime);
    this->clampPositionInMap();

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void SpyBalloon::render() const {
    PhysicsEntity::render();
}

void SpyBalloon::handleCollision(PhysicsEntity::Collision collision) {
    // Collided with a tile
    this->landedSuccessfully = true;
}