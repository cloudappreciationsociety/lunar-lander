#include "military.h"

#include "spy_balloon.h"
#include "texture.h"

FighterJet::FighterJet() : PhysicsEntity(TextureAsset::get("fighter_jet")) {}

void FighterJet::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }
    if (static_cast<float>(GetTime()) > this->lastFiredTime + this->fireRate) {}

    this->collideAndMove(deltaTime);

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void FighterJet::fireMissiles() const {}

void FighterJet::handleCollision(PhysicsEntity::Collision collision) {
    if (SpyBalloon* balloon = dynamic_cast<SpyBalloon*>(collision.entity)) {
        // If we hit the balloon, die.
        balloon->takeDamage(this->health, this->position);
        this->markForDeletion();
    }
}

void AntiAircraftGun::update(float deltaTime) {}

void AntiAircraftGun::handleCollision(PhysicsEntity::Collision collision) {}

Missile::Missile(Vector2 direction, float speed) : PhysicsEntity(TextureAsset::get("missile")) {}

void Missile::handleCollision(PhysicsEntity::Collision collision) {
    if (SpyBalloon* balloon = dynamic_cast<SpyBalloon*>(collision.entity)) {
        balloon->takeDamage(this->damage, this->position);
    }

    this->markForDeletion();
}

Explosion::Explosion() : Entity(TextureAsset::get("explosion")) {}