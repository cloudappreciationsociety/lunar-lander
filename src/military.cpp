#include "military.h"

#include "constants.h"
#include "spy_balloon.h"
#include "texture.h"

FighterJet::FighterJet() : MilitaryVehicle(TextureAsset::get("fighter_jet")) {
    this->health = FighterJet::BASE_HEALTH;
}

void FighterJet::explode() {
    auto explosion = Entity::create(new Explosion(this->collisionCandidates, 100.0f));
    explosion->position = this->position;
    explosion->damage = 75.0f;
    explosion->setDuration(0.5f);
    explosion->explode();

    this->markForDeletion();
}

void FighterJet::takeDamage(float damage, Vector2 damageOrigin) {
    // Ignore all knockback
    if (this->isAlive()) {
        Actor::takeDamage(damage);
    }
}

void FighterJet::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    float now = static_cast<float>(GetTime());

    if (!this->isAlive()) {
        this->acceleration.y = constants::G_ACCEL;
    } else {
        // Start a volley
        if (!this->isFiringVolley && now >= this->lastVolleyFireTime + this->fireRate) {
            this->isFiringVolley = true;
            this->currentMissilesFired = 0;
        }

        // Fire the next missile in the volley
        if (this->isFiringVolley && now >= this->lastMissileFireTime + this->missileFireRate) {
            Vector2 direction = Vector2Subtract(this->target->position, this->position);

            // Faster jets = faster missiles
            auto missile = Entity::create(
                new Missile(this->position, direction, 250.0f + Vector2Length(this->velocity))
            );
            missile->ownerId = this->id;
            missile->acceleration = {0.0f, 0.0f};
            missile->scale = {32.0f, 20.0f};
            missile->colliderDimensions = missile->scale;
            missile->blastRadius = 60.0f;
            missile->damage = FighterJet::BASE_MISSILE_DAMAGE;

            ++this->currentMissilesFired;
            this->lastMissileFireTime = now;
        }

        // Volley finished
        if (this->isFiringVolley && this->currentMissilesFired >= this->volleyMissileCount) {
            this->isFiringVolley = false;
            this->lastVolleyFireTime = now;
        }
    }

    this->collideAndMove(deltaTime);

    // Explosions and collisions may have set our velocity to 0 or below our cruising speed.
    // So maintain the base speed after the movement update.
    if (this->cruiseVelocityX > 0.0f) {
        this->velocity.x = std::max(this->cruiseVelocityX, this->velocity.x);
    } else {
        this->velocity.x = std::min(this->cruiseVelocityX, this->velocity.x);
    }

    // Delete ourselves if we cross map bounds.
    if (this->isOutOfBounds()) {
        this->markForDeletion();
        return;
    }

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void FighterJet::handleCollision(PhysicsEntity::Collision collision) {
    if (auto balloon = dynamic_cast<SpyBalloon*>(collision.entity)) {
        // If we hit the balloon, blow up.
        this->takeDamage(this->health, {0.0f, 0.0f});
        this->explode();

    } else if (auto military = dynamic_cast<MilitaryVehicle*>(collision.entity)) {
        if (military->isStationary() && military->health < this->health) {
            military->takeDamage(military->health, {0.0f, 0.0f});
            military->explode();
        } else if (military->health >= this->health || !this->isAlive()) {
            this->takeDamage(this->health, {0.0f, 0.0f});
            this->explode();
        }
    } else if (collision.entity->anchored) { // Hit the ground; blow up
        this->explode();
    }
}

AntiAircraftGun::AntiAircraftGun() :
    MilitaryVehicle(TextureAsset::get("aa_gun")),
    lastFireTime(static_cast<float>(GetTime())) {
    this->health = AntiAircraftGun::BASE_HEALTH;
}

void AntiAircraftGun::explode() {
    auto explosion = Entity::create(new Explosion(this->collisionCandidates, 200.0f));
    explosion->position = this->position;
    explosion->damage = 150.0f;
    explosion->setTexture(TextureAsset::get("ground_explosion"));
    explosion->explode();

    this->markForDeletion();
}

void AntiAircraftGun::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    float now = static_cast<float>(GetTime());

    if (now >= this->lastFireTime + this->fireRate) {
        Vector2 direction = Vector2Subtract(this->target->position, this->position);

        // Fire faster missiles if the target is in the top-half of the screen.
        float missileSpeed =
            (this->target->position.y <= constants::SCREEN_CENTER.y) ? 500.0f : 400.0f;

        auto missile = Entity::create(new Missile(this->position, direction, missileSpeed));
        missile->ownerId = this->id;
        missile->acceleration = {0.0f, constants::G_ACCEL};
        missile->scale = {65.0f, 45.0f};
        missile->colliderDimensions = {missile->scale.y, missile->scale.y};
        missile->blastRadius = 80.0f;
        missile->damage = 100.0f;
        missile->texture.loadAsset(TextureAsset::get("shell"));

        this->lastFireTime = now;
    }

    if (this->isMovingToGoal) {
        float remaining = this->moveGoalX - this->position.x;
        float step = this->moveSpeed * deltaTime;

        // Arrived: snap onto the goal
        if (std::fabs(remaining) <= step) {
            this->position.x = this->moveGoalX;
            this->velocity.x = 0.0f;
            this->isMovingToGoal = false;
        } else {
            this->velocity.x = (remaining > 0.0f) ? this->moveSpeed : -this->moveSpeed;
        }
    }

    this->collideAndMove(deltaTime);

    // If we're at our goal and get knocked by an explosion, apply some friction so we don't
    // slowly slide off the map
    if (!this->isMovingToGoal) {
        this->applyDragX(deltaTime, 0.5f);
    }

    if (this->isOutOfBounds()) {
        this->markForDeletion();
    }

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void AntiAircraftGun::render() const {
    MilitaryVehicle::render();
}

void AntiAircraftGun::handleCollision(PhysicsEntity::Collision collision) {
    if (auto balloon = dynamic_cast<SpyBalloon*>(collision.entity)) {
        // If we hit the balloon, blow up.
        this->takeDamage(this->health, {0.0f, 0.0f});
        this->explode();

    } else if (auto military = dynamic_cast<MilitaryVehicle*>(collision.entity)) {
        if (military->isStationary() && military->health < this->health) {
            military->takeDamage(military->health, {0.0f, 0.0f});
            military->explode();
        } else if (military->health >= this->health || !this->isAlive()) {
            this->takeDamage(this->health, {0.0f, 0.0f});
            this->explode();
        }
    }
}

Missile::Missile(Vector2 position, Vector2 direction, float speed) :
    PhysicsEntity(TextureAsset::get("missile")) {
    float radians = std::atan2(direction.y, direction.x);

    this->position = position;
    this->angle = radians * RAD2DEG;
    this->velocity = {speed * std::cos(radians), speed * std::sin(radians)};
}

// Don't collide with who fired us.
bool Missile::canCollideWith(const PhysicsEntity* other) const {
    return PhysicsEntity::canCollideWith(other) && other->id != this->ownerId;
}

void Missile::handleCollision(PhysicsEntity::Collision collision) {
    auto explosion = Entity::create(new Explosion(this->collisionCandidates, this->blastRadius));
    explosion->position = this->position;
    explosion->damage = this->damage;
    explosion->setDuration(0.33f);
    explosion->explode();

    this->markForDeletion();
}

void Missile::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    this->collideAndMove(deltaTime);

    this->angle = std::atan2(this->velocity.y, this->velocity.x) * RAD2DEG;

    // Delete ourselves if we cross map bounds
    if (this->isOutOfBounds()) {
        this->markForDeletion();
    }

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

Explosion::Explosion(const std::vector<PhysicsEntity*>& damageCandidates, float radius) :
    Entity(TextureAsset::get("explosion")),
    radius(radius),
    damageCandidates(damageCandidates) {
    this->scale = {2 * radius, 2 * radius};
    this->setDuration(this->duration);
}

void Explosion::setTexture(TextureAsset* asset) {
    this->texture.loadAsset(asset);
    this->texture.setDuration(this->duration);
}

void Explosion::setDuration(float duration) {
    this->duration = duration;
    this->texture.setDuration(this->duration);
}

void Explosion::explode() {
    for (auto entity : this->damageCandidates) {
        if (entity->anchored || entity->isMarkedForDeletion()) {
            continue;
        }

        Vector2 differenceVec = Vector2Subtract(entity->position, this->position);
        float distance = Vector2Length(differenceVec);
        if (distance > this->radius) {
            continue;
        }

        Vector2 direction = Vector2Normalize(differenceVec);

        float knockbackStrength = this->damage;
        Vector2 knockbackVector = Vector2Scale(direction, knockbackStrength);

        if (auto actor = dynamic_cast<Actor*>(entity)) {
            actor->takeDamage(this->damage, knockbackVector);
        } else if (auto proj = dynamic_cast<Missile*>(entity)) {
            entity->velocity = Vector2Add(entity->velocity, knockbackVector);
        }
    }
}

void Explosion::update(float deltaTime) {
    Entity::update(deltaTime);

    if (this->duration <= 0.0f) {
        this->markForDeletion();
        return;
    }

    this->duration -= deltaTime;
}

Military::Military(Actor* target) :
    randomEngine(std::random_device {}()),
    randomDist(0.0f, 1.0f),
    target(target) {}

FighterJet* Military::spawnJet() {
    FighterJet* jet = Entity::create(new FighterJet);

    bool spawnAtLeft = this->getRandomFloat() < 0.5f;
    jet->position.x = spawnAtLeft ? 0.0f : constants::SCREEN_WIDTH;
    jet->angle = spawnAtLeft ? 180.0f : 0.0f;

    jet->position.y =
        lerp(Military::JET_SPAWN_Y_MIN, Military::JET_SPAWN_Y_MAX, this->getRandomFloat());

    jet->acceleration.y = 0.0f;
    jet->velocity.x =
        lerp(Military::JET_MIN_SPEED, Military::JET_MAX_SPEED, this->getRandomFloat());

    jet->scale = {100.0f, 100.0f};
    jet->colliderDimensions = {jet->scale.x, jet->scale.y / 3.0f};

    if (!spawnAtLeft) {
        jet->velocity.x *= -1;
    }

    jet->cruiseVelocityX = jet->velocity.x;

    jet->target = this->target;

    return jet;
}

AntiAircraftGun* Military::spawnAAGun() {
    AntiAircraftGun* aaGun = Entity::create(new AntiAircraftGun);

    aaGun->scale = {150.0f, 150.0f};
    aaGun->colliderDimensions = {aaGun->scale.x, aaGun->scale.y / 1.5f};
    aaGun->target = this->target;
    aaGun->acceleration.y = constants::G_ACCEL;

    // Goal: somewhere near the center, within +/- 25% of the screen width
    float spread = constants::SCREEN_WIDTH * 0.25f;
    aaGun->moveGoalX = lerp(
        constants::SCREEN_CENTER.x - spread,
        constants::SCREEN_CENTER.x + spread,
        this->getRandomFloat()
    );
    aaGun->moveSpeed = lerp(25.0f, 45.0f, this->getRandomFloat());
    aaGun->isMovingToGoal = true;

    bool spawnAtLeft = this->getRandomFloat() < 0.5f;
    aaGun->position = {spawnAtLeft ? 0.0f : static_cast<float>(constants::SCREEN_WIDTH), 1100.0f};

    return aaGun;
}

void Military::update(float deltaTime) {
    projectileCollisionCandidates.clear();
    vehicleCollisionCandidates.clear();

    for (auto& pair : Entity::registry()) {
        Entity* ptr = pair.second.get();
        if (auto physEntity = dynamic_cast<PhysicsEntity*>(ptr)) {
            projectileCollisionCandidates.push_back(physEntity);

            if (dynamic_cast<Missile*>(physEntity) == nullptr) {
                vehicleCollisionCandidates.push_back(physEntity);
            }
        }
    }

    float now = static_cast<float>(GetTime());
    if (now < this->lastSpawnTime + this->SPAWN_INTERVAL) {
        return;
    }

    float spawnRoll = this->getRandomFloat();
    if (spawnRoll < Military::JET_SPAWN_THRESHOLD) {
        this->spawnJet();
        this->lastSpawnTime = now;
    }

    if (spawnRoll < Military::AA_GUN_SPAWN_THRESHOLD) {
        this->spawnAAGun();
        this->lastSpawnTime = now;
    }
}