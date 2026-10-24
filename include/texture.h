#ifndef TEXTURE_H
#define TEXTURE_H

#include <memory>

#include "lib.h"

enum class TextureType { SINGLE, ATLAS };

// Entites do not own but are rather assigned textures.
// TextureAsset is responsible for loading and unloading raylib textures.
class TextureAsset {
private:
    Texture2D texture; // Immutable once loaded
    const std::string alias;

public:
    TextureType type = TextureType::SINGLE;
    std::map<int32_t, std::vector<int32_t>> animationSpriteSheetFrames;
    Vector2 spriteSheetDimensions;

public:
    // Similar class-centralized registry spiel to Entity
    static std::map<std::string, std::unique_ptr<TextureAsset>>& registry();
    static TextureAsset* get(const std::string& alias);

    template<typename T>
    static T* create(T* object) {
        auto ptr = std::unique_ptr<T>(object);
        T* outPtr = ptr.get();
        TextureAsset::registry().emplace(ptr->alias, std::move(ptr));
        return outPtr;
    }

    TextureAsset(const std::string& alias, const std::string& pathToTexture);
    ~TextureAsset();

    void useDefaultAnimation();

    const Texture2D& getTexture2D() const {
        return this->texture;
    }

    Rectangle getTextureArea(int32_t sliceIndex) const;
};

class EntityTexture {
private:
    static constexpr uint32_t DEFAULT_FRAME_SPEED = 14;

private:
    const TextureAsset* asset = nullptr;

public:
    Vector2 uvOrigin = {0.0f, 0.0f};
    Vector2 uvSize;
    Color tint = WHITE;

    // For animations
    int32_t currentAnimation = 0;
    int32_t currentAnimationFrameIndex = 0;

    uint32_t frameSpeed = EntityTexture::DEFAULT_FRAME_SPEED;
    float animationTime = 0.0f;

private:
    const std::vector<int32_t>& getCurrentAnimationFrames() const {
        return this->asset->animationSpriteSheetFrames.at(this->currentAnimation);
    }

public:
    EntityTexture() = default;
    EntityTexture(const TextureAsset* asset);

    void loadAsset(const TextureAsset* asset);

    void render(Rectangle destination, Vector2 origin, float angle) const;

    void animate(float deltaTime);

    void setDuration(float duration) {
        this->frameSpeed =
            static_cast<uint32_t>(this->getCurrentAnimationFrames().size() / duration);
    }
};

#endif