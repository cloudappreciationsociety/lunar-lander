/**
* Author: Alan Chen
* Assignment: Lunar Lander
* Date due: 10/26/2026
*
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include <map>
#include <memory>
#include <random>

#include "constants.h"
#include "entity.h"
#include "lib.h"
#include "spy_balloon.h"
#include "ui.h"

enum class GameStatus { IN_MENU, STARTING, IN_PROGRESS, ENDED };

AppStatus gAppStatus = AppStatus::RUNNING;
GameStatus gGameStatus = GameStatus::IN_PROGRESS;

float gTimeAccumulator = 0.0f;
float gPreviousTimestampSec = 0.0f;

Color gBackgroundColor = SKYBLUE;

float gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

std::random_device randomDevice;
std::mt19937 generator(randomDevice());
std::uniform_real_distribution<float> puck_rng(0.0f, 1.0f);

TextButton gBackToMenuButton;
TextButton gPlayButton;
TextButton gQuitButton;

TextureAsset* gGrassTileTexture = nullptr;
SpyBalloon* gSpyBalloon = nullptr;
std::vector<PhysicsEntity*> gMapTiles;

void resetGame() {
    gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

    // Delete all pucks
}

void initialize() {
    InitWindow(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, "Save the Chinese Spy Balloon");

    SetTargetFPS(constants::TARGET_FPS);

    // Textures
    TextureAsset* spyBalloonTexture =
        TextureAsset::create(new TextureAsset("spy_balloon", "assets/textures/ai.png"));
    spyBalloonTexture->type = TextureType::SINGLE;

    gGrassTileTexture =
        TextureAsset::create(new TextureAsset("grass_tile", "assets/textures/grass-tile.png"));
    gGrassTileTexture->type = TextureType::SINGLE;

    TextureAsset* explosionTexture =
        TextureAsset::create(new TextureAsset("explosion", "assets/textures/explosion.png"));
    explosionTexture->type = TextureType::ATLAS;
    explosionTexture->spriteSheetDimensions = {4, 4};

    TextureAsset* fighterJetTexture =
        TextureAsset::create(new TextureAsset("fighter_jet", "assets/textures/fighter-jet.png"));
    fighterJetTexture->type = TextureType::ATLAS;
    fighterJetTexture->spriteSheetDimensions = {4, 1};

    TextureAsset* missileTexture =
        TextureAsset::create(new TextureAsset("missile", "assets/textures/missile.png"));
    missileTexture->type = TextureType::ATLAS;
    missileTexture->spriteSheetDimensions = {3, 1};

    // Entities
    gSpyBalloon = Entity::create(new SpyBalloon);
    gSpyBalloon->position = constants::SCREEN_CENTER;
    gSpyBalloon->position.y = 100.0f;

    gSpyBalloon->scale = {50.0f, 50.0f};
    gSpyBalloon->colliderDimensions = Vector2Scale(gSpyBalloon->scale, 1.0f);
    gSpyBalloon->acceleration.y = constants::G_ACCEL;

    Vector2 tileSize = {50.0f, 50.0f};
    int32_t tileCount = constants::SCREEN_WIDTH / tileSize.x;

    float leftMostX = constants::SCREEN_CENTER.x - (tileCount * tileSize.x) / 2.0f;

    for (int32_t i = 0; i < tileCount; ++i) {
        auto tile = Entity::create(new PhysicsEntity(gGrassTileTexture));
        tile->scale = tileSize;
        tile->colliderDimensions = tile->scale;
        tile->position = {leftMostX + i * tileSize.x, constants::SCREEN_HEIGHT - tileSize.y / 2.0f};

        gMapTiles.push_back(tile);
    }

    // Backgrounds
    // Sets player positions, zeroes out scores, etc.
    resetGame();

    // Audio

    // UI elements
    gBackToMenuButton.position = {constants::SCREEN_CENTER.x, 1.35f * constants::SCREEN_CENTER.y};
    gBackToMenuButton.size = {200.0f, 75.0f};
    gBackToMenuButton.backgroundColor = DARKGREEN;
    gBackToMenuButton.hoverColor = GREEN;
    gBackToMenuButton.text = "To Menu";

    gPlayButton.position = {constants::SCREEN_CENTER.x, 1.35f * constants::SCREEN_CENTER.y};
    gPlayButton.size = {200.0f, 75.0f};
    gPlayButton.backgroundColor = DARKGREEN;
    gPlayButton.hoverColor = GREEN;
    gPlayButton.text = "PLAY";

    gQuitButton.position = {constants::SCREEN_CENTER.x, 1.6f * constants::SCREEN_CENTER.y};
    gQuitButton.size = {200.0f, 75.0f};
    gQuitButton.backgroundColor = RED;
    gQuitButton.hoverColor = PINK;
    gQuitButton.text = "QUIT";

    gPreviousTimestampSec = static_cast<float>(GetTime());
}

void processInput() {
    Vector2 mousePosition = GetMousePosition();

    switch (gGameStatus) {
        case (GameStatus::IN_MENU): {
            if (gPlayButton.leftClicked()) {
                resetGame();
                gGameStatus = GameStatus::STARTING;
            }

            if (gQuitButton.leftClicked()) {
                gAppStatus = AppStatus::TERMINATED;
            }

            break;
        }
        case (GameStatus::STARTING): {
            // No inputs possible
            break;
        }
        case (GameStatus::IN_PROGRESS): {
            gSpyBalloon->resetMoveDirection();

            if (IsKeyDown(KeyboardKey::KEY_W)) {
                gSpyBalloon->onInput(KeyboardKey::KEY_W);
            }

            if (IsKeyDown(KeyboardKey::KEY_A)) {
                gSpyBalloon->onInput(KeyboardKey::KEY_A);
            } else if (IsKeyDown(KeyboardKey::KEY_D)) {
                gSpyBalloon->onInput(KeyboardKey::KEY_D);
            }

            break;
        }
        case (GameStatus::ENDED): {
            if (gBackToMenuButton.leftClicked()) {
                resetGame();
                gGameStatus = GameStatus::IN_MENU;
            }

            if (gQuitButton.leftClicked()) {
                gAppStatus = AppStatus::TERMINATED;
            }

            break;
        }
        default:
            break;
    }

    if (IsKeyDown(KEY_Q) || WindowShouldClose()) {
        gAppStatus = AppStatus::TERMINATED;
    }
}

void updateGameMenu() {}

void updateGameStarting(float deltaTime) {
    gRoundCountdown = std::max(0.0f, gRoundCountdown - deltaTime);

    if (gRoundCountdown == 0.0f) {
        gGameStatus = GameStatus::IN_PROGRESS;
    }
}

void updateGameInProgress(float deltaTime) {
    for (auto& pair : Entity::registry()) {
        pair.second->update(deltaTime);
    }

    // Collisions
    gSpyBalloon->setCollisionCandidates(gMapTiles);

    for (auto& pair : Entity::registry()) {}
}

void update() {
    float nowSec = static_cast<float>(GetTime());
    float deltaTime = nowSec - gPreviousTimestampSec;
    gPreviousTimestampSec = nowSec;

    deltaTime += gTimeAccumulator; // fold in any leftover from last frame

    if (deltaTime < constants::FIXED_TIMESTEP) {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= constants::FIXED_TIMESTEP) {
        // Delete objects marked for deletion
        // Ugly C++11 iterate-and-erase stuff
        for (auto it = Entity::registry().begin(); it != Entity::registry().end();) {
            if (it->second->isMarkedForDeletion()) {
                it = Entity::registry().erase(it);
            } else {
                ++it;
            }
        }

        switch (gGameStatus) {
            case (GameStatus::IN_MENU): {
                updateGameMenu();
                break;
            }
            case (GameStatus::STARTING): {
                updateGameStarting(constants::FIXED_TIMESTEP);
                break;
            }
            case (GameStatus::IN_PROGRESS): {
                updateGameInProgress(constants::FIXED_TIMESTEP);
                break;
            }
            case (GameStatus::ENDED): {
                break;
            }
            default:
                break;
        }

        deltaTime -= constants::FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void renderTintedOverlay() {
    drawRectangle(constants::SCREEN_CENTER, constants::SCREEN_SIZE, constants::BLACK_TINT);
}

void render() {
    BeginDrawing();

    ClearBackground(gBackgroundColor);

    for (const auto& pair : Entity::registry()) {
        pair.second->render();
    }

    switch (gGameStatus) {
        case (GameStatus::IN_MENU): {
            gPlayButton.render();
            gQuitButton.render();

            break;
        }
        case (GameStatus::STARTING): {
            renderTintedOverlay();
            drawText(
                TextFormat(
                    "Match begins in %i seconds...",
                    static_cast<int32_t>(std::ceil(gRoundCountdown))
                ),
                constants::SCREEN_CENTER,
                constants::ui::HEADER_FONT_SIZE,
                WHITE
            );

            break;
        }
        case (GameStatus::IN_PROGRESS): {
            break;
        }
        case (GameStatus::ENDED): {
            renderTintedOverlay();

            gBackToMenuButton.render();
            gQuitButton.render();

            break;
        }
        default:
            break;
    }

    EndDrawing();
}

void shutdown() {
    Entity::registry().clear();
    TextureAsset::registry().clear();

    CloseWindow();
}

int main() {
    initialize();

    while (gAppStatus == AppStatus::RUNNING) {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}