#ifndef SPY_BALLOON_H
#define SPY_BALLOON_H

#include "entity.h"

struct DamageLog {
    static constexpr float LOG_TTL = 1.0f;

    struct Entry {
        float damage;
        Vector2 position;
        float timestamp;
    };

    std::vector<Entry> entries;
};

class ForceField: public PhysicsEntity {
public:
    float startTime = 0.0f;
    float duration = 3.0f;
    float radius = 10.0f;

public:
    ForceField();
};

class SpyBalloon: public Actor {
private:
    static constexpr float STARTING_FUEL = 400.0f;
    static constexpr float STARTING_HEALTH = 1000.0f;
    static constexpr float G_ACCEL_MULTIPLIER = 0.05f;
    static constexpr float DRAG = 2.0f;
    static constexpr float MOVE_ACCEL_X = 30.0f;

    static constexpr float MOVE_FUEL_DRAIN = -0.05f;

private:
    Vector2 moveDirection = {0.0f, 0.0f};

public:
    float fuel = SpyBalloon::STARTING_FUEL;
    bool landedSuccessfully = false;

    std::vector<std::pair<float, Vector2>> damageLog;

    float lastRepulsionShieldUseTime = 0.0f;

private:
    void addFuel(float amount) {
        this->fuel = std::max(0.0f, this->fuel + amount);
    }

    void clampPositionInMap();
    void activateRepulsionShield();

public:
    SpyBalloon();

    void resetMoveDirection();
    void onInput(KeyboardKey key);

    void update(float deltaTime) override;
    void render() const override;

    void handleCollision(PhysicsEntity::Collision collision) override;
};

#endif