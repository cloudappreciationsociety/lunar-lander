#ifndef SPY_BALLOON_H
#define SPY_BALLOON_H

#include "entity.h"

class SpyBalloon: public PhysicsEntity {
private:
    static constexpr float STARTING_FUEL = 400.0f;
    static constexpr float STARTING_HEALTH = 1000.0f;
    static constexpr float G_ACCEL_MULTIPLIER = 0.10f;
    static constexpr float DRAG = 2.0f;
    static constexpr float MOVE_ACCEL_X = 10.0f;

private:
    Vector2 moveDirection = {0.0f, 0.0f};

public:
    float fuel = SpyBalloon::STARTING_FUEL;
    float health = SpyBalloon::STARTING_HEALTH;

public:
    SpyBalloon();

    void resetMoveDirection();
    void onInput(KeyboardKey key);

    void update(float deltaTime) override;
    void render() const override;

    bool isAlive() const {
        return this->health >= 0.0f;
    }

    void takeDamage(float damage, Vector2 damageOrigin);
};

#endif