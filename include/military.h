#ifndef MILITARY_H
#define MILITARY_H

#include <random>

#include "entity.h"

class MilitaryVehicle: public Actor {
public:
    Actor* target = nullptr;

public:
    using Actor::Actor;

    virtual void explode() = 0;
};

class FighterJet: public MilitaryVehicle {
private:
    static constexpr float BASE_SPEED = 300.0f;
    static constexpr float BASE_FIRE_RATE = 2.0f;
    static constexpr float BASE_HEALTH = 100.0f;
    static constexpr float BASE_VOLLEY_FIRE_RATE = 0.35f;
    static constexpr uint32_t BASE_VOLLEY_FIRE_COUNT = 2;
    static constexpr float BASE_MISSILE_DAMAGE = 25.0f;

public:
    bool isFiringVolley = false;
    uint32_t volleyMissileCount = FighterJet::BASE_VOLLEY_FIRE_COUNT;
    uint32_t currentMissilesFired = 0;
    float missileFireRate = FighterJet::BASE_VOLLEY_FIRE_RATE;
    float lastMissileFireTime = 0.0f;

    float fireRate = FighterJet::BASE_FIRE_RATE;
    float lastVolleyFireTime = 0.0f;
    float cruiseVelocityX = 0.0f;

protected:
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    FighterJet();

    void explode() override;
    void takeDamage(float damage, Vector2 addVelocity) override;
    void update(float deltaTime) override;
};

class AntiAircraftGun: public MilitaryVehicle {
private:
    static constexpr float BASE_SPEED = 300.0f;
    static constexpr float BASE_FIRE_RATE = 5.0f;
    static constexpr float BASE_HEALTH = 150.0f;

public:
    float fireRate = AntiAircraftGun::BASE_FIRE_RATE;
    float lastFireTime = 0.0f;

    float moveSpeed = 75.0f;
    float moveGoalX = 0.0f;
    bool isMovingToGoal = false;

protected:
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    AntiAircraftGun();

    void explode() override;
    void update(float deltaTime) override;
    void render() const override;
};

class Missile: public PhysicsEntity {
private:
    static constexpr float BASE_DAMAGE = 50.0f;
    static constexpr float BASE_BLAST_RADIUS = 50.0f;

public:
    float damage = Missile::BASE_DAMAGE;
    float blastRadius = Missile::BASE_BLAST_RADIUS;

    // Who fired the missile.
    // Entity ID rather than pointer because it's possible our owner gets deleted before we do.
    uint32_t ownerId = -1;

protected:
    bool canCollideWith(const PhysicsEntity* other) const override;
    void handleCollision(PhysicsEntity::Collision collision) override;

public:
    Missile(Vector2 position, Vector2 direction, float speed);

    void update(float deltaTime) override;
};

class Explosion: public Entity {
private:
    static constexpr uint32_t FRAME_SPEED = 20;
    static constexpr float DURATION = 1.0f;
    static constexpr float ALPHA = 255 / 2;

private:
    float duration = Explosion::DURATION;

public:
    float radius = 0.0f;
    float damage = 0.0f;
    std::vector<PhysicsEntity*> damageCandidates;

public:
    Explosion(const std::vector<PhysicsEntity*>& damageCandidates, float radius);

    void setTexture(TextureAsset* asset);
    void setDuration(float duration);
    void explode();
    void update(float deltaTime) override;
};

class Military {
private:
    static constexpr float SPAWN_CHANCE = 0.10f;
    static constexpr float SPAWN_INTERVAL = 2.0f;

    static constexpr float JET_SPAWN_Y_MAX = 100.0f;
    static constexpr float JET_SPAWN_Y_MIN = 1000.0f;

    static constexpr float JET_SPAWN_THRESHOLD = 0.25f;

    static constexpr float JET_MAX_SPEED = 200.0f;
    static constexpr float JET_MIN_SPEED = 50.0f;

    static constexpr float AA_GUN_SPAWN_THRESHOLD = 0.15f;

private:
    Actor* target = nullptr;
    float lastSpawnTime = 0.0f;
    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> randomDist;
    float spawnInterval = Military::SPAWN_INTERVAL;

public:
    // What missiles can collide with
    std::vector<PhysicsEntity*> projectileCollisionCandidates;

    // What vehicles can collide with
    std::vector<PhysicsEntity*> vehicleCollisionCandidates;

private:
    float getRandomFloat() {
        return this->randomDist(this->randomEngine);
    }

    FighterJet* spawnJet();
    AntiAircraftGun* spawnAAGun();

public:
    Military(Actor* target);

    void update(float deltaTime);
};

#endif