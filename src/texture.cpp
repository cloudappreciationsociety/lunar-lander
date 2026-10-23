#include "texture.h"

std::map<std::string, std::unique_ptr<TextureAsset>>& TextureAsset::registry() {
    static std::map<std::string, std::unique_ptr<TextureAsset>> registry;
    return registry;
}

TextureAsset* TextureAsset::get(const std::string& alias) {
    return TextureAsset::registry().at(alias).get();
}

TextureAsset::TextureAsset(const std::string& alias, const std::string& pathToTexture) :
    alias(alias),
    texture(LoadTexture(pathToTexture.c_str())) {}

TextureAsset::~TextureAsset() {
    UnloadTexture(this->texture);
}

Rectangle TextureAsset::getTextureArea(int32_t sliceIndex) const {
    return getUVRectangle(
        &this->getTexture2D(),
        sliceIndex,
        this->spriteSheetDimensions.x,
        this->spriteSheetDimensions.y
    );
}

EntityTexture::EntityTexture(const TextureAsset* asset) {
    this->loadAsset(asset);
}

void EntityTexture::loadAsset(const TextureAsset* asset) {
    this->asset = asset;

    this->uvSize = {
        static_cast<float>(asset->getTexture2D().width),
        static_cast<float>(asset->getTexture2D().height)
    };

    // Unnecessary as this gets computed by TextureAsset::getTextureArea anyway
    if (this->asset->type == TextureType::ATLAS) {
        this->uvSize.x /= this->asset->spriteSheetDimensions.x;
        this->uvSize.y /= this->asset->spriteSheetDimensions.y;
    }

    this->animationTime = 0.0f;
    this->currentAnimationFrameIndex = 0;
}

void EntityTexture::render(Rectangle destination, Vector2 origin, float angle, Color tint) const {
    Rectangle textureArea;

    switch (this->asset->type) {
        case (TextureType::SINGLE): {
            textureArea = {this->uvOrigin.x, this->uvOrigin.y, this->uvSize.x, this->uvSize.y};
            break;
        }

        case (TextureType::ATLAS): {
            textureArea = this->asset->getTextureArea(
                this->getCurrentAnimationFrames()[this->currentAnimationFrameIndex]
            );
            break;
        }

        default:
            break;
    }

    DrawTexturePro(this->asset->getTexture2D(), textureArea, destination, origin, angle, tint);
}

void EntityTexture::animate(float deltaTime) {
    if (this->asset->type != TextureType::ATLAS) {
        return;
    }

    this->animationTime += deltaTime;
    float secondsPerFrame = 1.0f / this->frameSpeed;

    if (this->animationTime >= secondsPerFrame) {
        this->animationTime = 0.0f;

        this->currentAnimationFrameIndex++;
        this->currentAnimationFrameIndex %= this->getCurrentAnimationFrames().size();
    }
}