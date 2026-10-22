#ifndef SPY_BALLOON_H
#define SPY_BALLOON_H

#include "entity.h"

class SpyBalloon: public PhysicsEntity {
private:
    static constexpr float STARTING_FUEL = 400;
    static constexpr float STARTING_HEALTH = 1000;

public:
    float fuel = SpyBalloon::STARTING_FUEL;
    float health = SpyBalloon::STARTING_HEALTH;

public:
    using PhysicsEntity::PhysicsEntity;

    void onInput();

    void update(float deltaTime) override;
};

#endif