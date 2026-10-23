#include "entity.h"

uint32_t Entity::idCounter = 0;

std::map<uint32_t, std::unique_ptr<Entity>>& Entity::registry() {
    static std::map<uint32_t, std::unique_ptr<Entity>> registry;
    return registry;
}

Entity::Entity() : id(Entity::idCounter++) {}

Entity::Entity(const TextureAsset* textureAsset) : id(Entity::idCounter++), texture(textureAsset) {}

bool Entity::shouldAnimate() const {
    return true;
}

bool Entity::shouldUpdate() const {
    return this->status == EntityStatus::ACTIVE;
}

void Entity::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void Entity::render() const {
    if (this->status == EntityStatus::INACTIVE) {
        return;
    }

    // Destination rectangle – centred on position
    Rectangle destinationArea = {
        this->position.x,
        this->position.y,
        static_cast<float>(this->scale.x),
        static_cast<float>(this->scale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(this->scale.x) / 2.0f,
        static_cast<float>(this->scale.y) / 2.0f
    };

    // Render the texture on screen
    this->texture.render(destinationArea, originOffset, this->angle, WHITE);

    //this->displayCollider();
}

void PhysicsEntity::resetColliderFlags() {
    this->isCollidingTop = false;
    this->isCollidingBottom = false;
    this->isCollidingRight = false;
    this->isCollidingLeft = false;
}

/**
 * Checks if two entities are colliding based on their positions and collider
 * dimensions.
 *
 * @param other represents another Entity with which you want to check for
 * collision. It is a pointer to the Entity class.
 *
 * @return returns `true` if the two entities are colliding based on their
 * positions and collider dimensions, and `false` otherwise.
 */
bool PhysicsEntity::isColliding(const PhysicsEntity* other) const {
    float xDistance = fabs(this->position.x - other->position.x)
        - ((this->colliderDimensions.x + other->colliderDimensions.x) / 2.0f);
    float yDistance = fabs(this->position.y - other->position.y)
        - ((this->colliderDimensions.y + other->colliderDimensions.y) / 2.0f);

    return xDistance < 0.0f && yDistance < 0.0f;
}

void PhysicsEntity::checkCollisionsX() {
    for (PhysicsEntity* collidableEntity : this->collisionCandidates) {
        if (!this->isColliding(collidableEntity)) {
            continue;
        }

        // When standing on a platform, we're always slightly overlapping
        // it vertically due to gravity, which causes false horizontal
        // collision detections. So the solution I found is only resolve X
        // collisions if there's significant Y overlap, preventing the
        // platform we're standing on from acting like a wall.
        float yDistance = fabs(this->position.y - collidableEntity->position.y);
        float yOverlap = fabs(
            yDistance - (this->colliderDimensions.y / 2.0f)
            - (collidableEntity->colliderDimensions.y / 2.0f)
        );

        // Skip if barely touching vertically (standing on platform)
        if (yOverlap < PhysicsEntity::Y_COLLISION_THRESHOLD) {
            continue;
        }

        float xDistance = fabs(this->position.x - collidableEntity->position.x);
        float xOverlap = fabs(
            xDistance - (this->colliderDimensions.x / 2.0f)
            - (collidableEntity->colliderDimensions.x / 2.0f)
        );

        if (this->velocity.x > 0) {
            this->position.x -= xOverlap;
            this->velocity.x = 0;

            // Collision!
            this->isCollidingRight = true;

            this->collisions.push_back({collidableEntity, CollisionFace::RIGHT});
        } else if (this->velocity.x < 0) {
            this->position.x += xOverlap;
            this->velocity.x = 0;

            // Collision!
            this->isCollidingLeft = true;

            this->collisions.push_back({collidableEntity, CollisionFace::LEFT});
        }
    }
}

/**
 * Iterates through a list of collidable entities, checks for collisions with
 * the player entity, and resolves any vertical overlap by adjusting the
 * player's position and velocity accordingly.
 *
 * @param collidableEntities An array of pointers to `Entity` objects that
 * represent the entities that the current `Entity` instance can potentially
 * collide with. The `collisionCheckCount` parameter specifies the number of
 * entities in the `collidableEntities` array that need to be checked for
 * collision.
 * @param collisionCheckCount The number of entities that the current entity
 * (`Entity`) should check for collisions with. This parameter specifies how
 * many entities are in the `collidableEntities` array that need to be checked
 * for collisions with the current entity.
 */
void PhysicsEntity::checkCollisionsY() {
    // STEP 1: For every entity that our player can collide with...
    for (PhysicsEntity* collidableEntity : this->collisionCandidates) {
        if (!this->isColliding(collidableEntity)) {
            continue;
        }

        // STEP 2: Calculate the distance between its centre and our centre
        //         and use that to calculate the amount of overlap between
        //         both bodies.
        float yDistance = fabs(this->position.y - collidableEntity->position.y);
        float yOverlap = fabs(
            yDistance - (this->colliderDimensions.y / 2.0f)
            - (collidableEntity->colliderDimensions.y / 2.0f)
        );

        // STEP 3: "Unclip" ourselves from the other entity, and zero our
        //         vertical velocity.
        if (this->velocity.y > 0) {
            this->position.y -= yOverlap;
            this->velocity.y = 0;
            this->isCollidingBottom = true;

            this->collisions.push_back({collidableEntity, CollisionFace::BOTTOM});
        } else if (this->velocity.y < 0) {
            this->position.y += yOverlap;
            this->velocity.y = 0;
            this->isCollidingTop = true;

            this->collisions.push_back({collidableEntity, CollisionFace::TOP});
        }
    }
}

void PhysicsEntity::collideAndMove(float deltaTime) {
    this->resetColliderFlags();
    this->collisions.clear();

    this->velocity.x += this->acceleration.x * deltaTime;
    this->velocity.y += this->acceleration.y * deltaTime;

    this->position.y += this->velocity.y * deltaTime;
    this->checkCollisionsY();

    this->position.x += this->velocity.x * deltaTime;
    this->checkCollisionsX();

    for (const auto& collision : this->collisions) {
        this->handleCollision(collision);
    }
}

void PhysicsEntity::displayCollider() const {
    // draw the collision box
    Rectangle colliderBox = {
        this->position.x - this->colliderDimensions.x / 2.0f,
        this->position.y - this->colliderDimensions.y / 2.0f,
        this->colliderDimensions.x,
        this->colliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x, // Top-left X
        colliderBox.y, // Top-left Y
        colliderBox.width, // Width
        colliderBox.height, // Height
        GREEN // Color
    );
}

void PhysicsEntity::setCollisionCandidates(const std::vector<PhysicsEntity*>& entities) {
    this->collisionCandidates.clear();

    for (auto e : entities) {
        this->collisionCandidates.push_back(e);
    }
}

void PhysicsEntity::update(float deltaTime) {
    if (!this->shouldUpdate()) {
        return;
    }

    this->collideAndMove(deltaTime);

    if (this->shouldAnimate()) {
        this->texture.animate(deltaTime);
    }
}

void PhysicsEntity::render() const {
    Entity::render();

    this->displayCollider();
}