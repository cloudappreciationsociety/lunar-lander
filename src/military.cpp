#include "military.h"

#include "constants.h"
#include "spy_balloon.h"
#include "texture.h"

FighterJet::FighterJet() : MilitaryVehicle(TextureAsset::get("fighter_jet")) {
    this->health = FighterJet::BASE_HEALTH;
}

void FighterJet::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    float now = static_cast<float>(GetTime());

    // Start a volley
    if (!this->isFiringVolley && now >= this->lastVolleyFireTime + this->fireRate) {
        this->isFiringVolley = true;
        this->currentMissilesFired = 0;
    }

    // Fire the next missile in the volley
    if (this->isFiringVolley && now >= this->lastMissileFireTime + this->missileFireRate) {
        Vector2 direction = Vector2Subtract(this->target->position, this->position);

        auto missile = Entity::create(new Missile(this->position, direction, 300.0f));
        missile->ownerId = this->id;
        missile->acceleration = {0.0f, 0.0f};
        missile->scale = {32.0f, 20.0f};
        missile->colliderDimensions = missile->scale;

        ++this->currentMissilesFired;
        this->lastMissileFireTime = now;
    }

    // Volley finished
    if (this->isFiringVolley && this->currentMissilesFired >= this->volleyMissileCount) {
        this->isFiringVolley = false;
        this->lastVolleyFireTime = now;
    }

    this->collideAndMove(deltaTime);

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
        balloon->takeDamage(this->health, this->position);
        this->markForDeletion();

        auto explosion = Entity::create(new Explosion(this->collisionCandidates, 100.0f));
        explosion->position = this->position;
        explosion->damage = 75.0f;
    } else if (auto military = dynamic_cast<MilitaryVehicle*>(collision.entity)) {
        // If we hit an ally, blow up if they have more or equal health than us.
        if (military->health >= this->health) {
            this->markForDeletion();

            auto explosion = Entity::create(new Explosion(this->collisionCandidates, 100.0f));
            explosion->position = this->position;
            explosion->damage = 75.0f;
        }
    }
}

void AntiAircraftGun::update(float deltaTime) {}

void AntiAircraftGun::handleCollision(PhysicsEntity::Collision collision) {}

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
    if (auto balloon = dynamic_cast<SpyBalloon*>(collision.entity)) {
        balloon->takeDamage(this->damage, this->position);
    } else if (auto military = dynamic_cast<MilitaryVehicle*>(collision.entity)) {
        std::cout << "HIT MILITARY\n";
    }

    this->markForDeletion();

    auto explosion = Entity::create(new Explosion(this->collisionCandidates, 50.0f));
    explosion->position = this->position;
    explosion->damage = this->damage;
    explosion->setDuration(0.33f);
}

void Missile::explode() {}

void Missile::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    this->collideAndMove(deltaTime);

    // Delete ourselves if we cross map bounds.
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

void Explosion::setDuration(float duration) {
    this->duration = duration;
    this->texture.setDuration(this->duration);
}

void Explosion::explode() {
    for (auto entity : this->damageCandidates) {
        if (Vector2DistanceSqr(entity->position, this->position) > this->radius * this->radius) {
            continue;
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

Military::Military(PhysicsEntity* target) :
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

    jet->target = this->target;

    return jet;
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
}