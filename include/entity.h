#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <memory>

#include "lib.h"
#include "texture.h"

enum class Direction { LEFT, UP, RIGHT, DOWN }; // For movement
enum class CollisionFace { TOP, BOTTOM, LEFT, RIGHT };

enum class EntityStatus { ACTIVE, INACTIVE };

class Entity {
private:
    static constexpr int DEFAULT_SIZE = 250;

    static uint32_t idCounter;

protected:
    bool shouldDelete = false;

public:
    const uint32_t id;
    Vector2 position = {0.0f, 0.0f};
    Vector2 scale = {Entity::DEFAULT_SIZE, Entity::DEFAULT_SIZE};
    float angle = 0.0f;

    EntityTexture texture;
    EntityStatus status = EntityStatus::ACTIVE;

protected:
    virtual bool shouldAnimate() const;

public:
    // Returns a map of all currently active objects
    // std::map over std::unordered_map to guarantee rendering order so BG renders
    // behind everything else
    static std::map<uint32_t, std::unique_ptr<Entity>>& registry();

    template<typename T>
    static T* create(T* object) {
        auto ptr = std::unique_ptr<T>(object);
        T* outPtr = ptr.get();
        Entity::registry().emplace(ptr->id, std::move(ptr));
        return outPtr;
    }

    Entity();
    Entity(const TextureAsset* textureAsset);

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    virtual bool shouldUpdate() const;
    virtual void update(float deltaTime);
    virtual void render() const;

    const bool isMarkedForDeletion() const {
        return this->shouldDelete;
    }

    void markForDeletion() {
        this->shouldDelete = true;
    }

    void activate() {
        this->status = EntityStatus::ACTIVE;
    }

    void deactivate() {
        this->status = EntityStatus::INACTIVE;
    }

    bool isActive() {
        return this->status == EntityStatus::ACTIVE;
    }
};

// An entity that interacts with the world, physically
class PhysicsEntity: public Entity {
private:
    static constexpr float Y_COLLISION_THRESHOLD = 0.5f;

public:
    struct Collision {
        PhysicsEntity* entity;
        CollisionFace face;
    };

protected:
    bool isCollidingTop = false;
    bool isCollidingBottom = false;
    bool isCollidingRight = false;
    bool isCollidingLeft = false;

    std::vector<PhysicsEntity*> collisionCandidates;
    std::vector<Collision> collisions;

public:
    Vector2 velocity = {0.0f, 0.0f};
    Vector2 acceleration = {0.0f, 0.0f};
    Vector2 colliderDimensions = {0.0f, 0.0f};

    // If true, this object will not move in collideAndMove.
    bool anchored = false;

private:
    void resetColliderFlags();
    bool isColliding(const PhysicsEntity* other) const;
    void checkCollisionsX();
    void checkCollisionsY();

protected:
    void collideAndMove(float deltaTime);
    void applyDragX(float deltaTime, float dragCoefficient);

    void displayCollider() const;

    bool isOutOfBounds() const;

    virtual bool canCollideWith(const PhysicsEntity* other) const {
        return other != this;
    }

    virtual void handleCollision(Collision collision) {
        return;
    }

public:
    using Entity::Entity;

    void setCollisionCandidates(const std::vector<PhysicsEntity*>& entities);
    void update(float deltaTime) override;
    void render() const override;

    bool isStationary() const {
        return Vector2Length(this->velocity) < 0.001f;
    }
};

// An entity with health
class Actor: public PhysicsEntity {
public:
    float health = 0.0f;

public:
    using PhysicsEntity::PhysicsEntity;

    bool isAlive() const {
        return this->health > 0.0f;
    }

    virtual void takeDamage(float damage) {
        this->health = std::max(0.0f, this->health - damage);
    }

    virtual void takeDamage(float damage, Vector2 addVelocity) {
        this->takeDamage(damage);
        this->velocity = Vector2Add(this->velocity, addVelocity);
    }
};

#endif // ENTITY_H