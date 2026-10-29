#ifndef SPY_BALLOON_H
#define SPY_BALLOON_H

#include "entity.h"

class ForceField: public PhysicsEntity {
public:
    using PhysicsEntity::PhysicsEntity;
};

class SpyBalloon: public Actor {
private:
    static constexpr float STARTING_FUEL = 400.0f;
    static constexpr float STARTING_HEALTH = 1000.0f;
    static constexpr float G_ACCEL_MULTIPLIER = 0.05f;
    static constexpr float DRAG = 2.0f;
    static constexpr float MOVE_ACCEL_X = 30.0f;

    static constexpr float MOVE_FUEL_DRAIN = 0.08f;

    static constexpr float FORCE_FIELD_FUEL_COST = 100.0f;
    static constexpr float FORCE_FIELD_COOLDOWN = 10.0f;
    static constexpr float FORCE_FIELD_DURATION = 3.0f;

private:
    Vector2 moveDirection = {0.0f, 0.0f};

public:
    float fuel = SpyBalloon::STARTING_FUEL;
    float maxFuel = SpyBalloon::STARTING_FUEL;
    bool landedSuccessfully = false;

    std::vector<std::pair<float, Vector2>> damageLog;

    float lastForceFieldUseTime = 0.0f;
    float forceFieldCooldown = SpyBalloon::FORCE_FIELD_COOLDOWN;
    float forceFieldDuration = SpyBalloon::FORCE_FIELD_DURATION;
    ForceField* forceField = nullptr;

private:
    void addFuel(float amount) {
        this->fuel = std::min(this->maxFuel, std::max(0.0f, this->fuel + amount));
    }

    void clampPositionInMap();
    void activateForceField();

public:
    SpyBalloon();

    void resetMoveDirection();
    void onInput(KeyboardKey key);

    void update(float deltaTime) override;
    void render() const override;

    void takeDamage(float damage, Vector2 addVelocity) override;
    void handleCollision(PhysicsEntity::Collision collision) override;
};

#endif