#ifndef MILITARY_H
#define MILITARY_H

#include "entity.h"

class FighterJet: public PhysicsEntity {
private:
    static constexpr float BASE_SPEED = 300.0f;
    static constexpr float BASE_FIRE_RATE = 0.25f;
    static constexpr float STARTING_HEALTH = 100.0f;

public:
    float fireRate = FighterJet::BASE_FIRE_RATE;
    float lastFiredTime = 0.0f;
    float health = FighterJet::STARTING_HEALTH;

private:
    void fireMissiles() const;

protected:
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    FighterJet();

    virtual void update(float deltaTime);
};

class AntiAircraftGun: public PhysicsEntity {
private:
    static constexpr float BASE_SPEED = 300.0f;
    static constexpr float BASE_FIRE_RATE = 5.0f;

public:
    float fireRate = AntiAircraftGun::BASE_FIRE_RATE;
    float lastFiredTime = 0.0f;

protected:
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    virtual void update(float deltaTime);
};

class Missile: public PhysicsEntity {
private:
    static constexpr float BASE_DAMAGE = 50.0f;

public:
    float damage = Missile::BASE_DAMAGE;

protected:
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    Missile(Vector2 direction, float speed);
};

class Explosion: public Entity {
public:
    float radius;
    float damage;

public:
    Explosion();
};

#endif