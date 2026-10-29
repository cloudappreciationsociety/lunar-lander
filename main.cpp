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
#include "military.h"
#include "spy_balloon.h"
#include "ui.h"

enum class GameStatus { IN_MENU, STARTING, IN_PROGRESS, ENDED };

AppStatus gAppStatus = AppStatus::RUNNING;
GameStatus gGameStatus = GameStatus::IN_MENU;

float gTimeAccumulator = 0.0f;
float gPreviousTimestampSec = 0.0f;

Color gBackgroundColor = SKYBLUE;

float gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

TextButton gBackToMenuButton;
TextButton gPlayButton;
TextButton gQuitButton;

ProgressBar gHealthBar;
ProgressBar gFuelBar;
AbilityStatus gForceFieldStatus;

TextureAsset* gGrassTileTexture = nullptr;
SpyBalloon* gSpyBalloon = nullptr;
std::vector<PhysicsEntity*> gMapTiles;
Military* gMilitary = nullptr;

void resetGame() {
    gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

    gSpyBalloon->position = constants::SCREEN_CENTER;
    gSpyBalloon->position.y = 100.0f;
    gSpyBalloon->health = gSpyBalloon->maxHealth;
    gSpyBalloon->fuel = gSpyBalloon->maxFuel;
    gSpyBalloon->acceleration.y = constants::G_ACCEL;
    gSpyBalloon->velocity = {0.0f, 0.0f};
    gSpyBalloon->landedSuccessfully = false;
    gSpyBalloon->lastForceFieldUseTime = -gSpyBalloon->forceFieldCooldown;

    // Delete
    for (auto& pair : Entity::registry()) {
        auto ptr = pair.second.get();

        if (dynamic_cast<MilitaryVehicle*>(ptr) || dynamic_cast<Missile*>(ptr)
            || dynamic_cast<Explosion*>(ptr)) {
            ptr->markForDeletion();
        }
    }

    if (gSpyBalloon->forceField) {
        gSpyBalloon->forceField->markForDeletion();
        gSpyBalloon->forceField = nullptr;
    }
}

void initialize() {
    InitWindow(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, "Save the Chinese Spy Balloon");

    SetTargetFPS(constants::TARGET_FPS);

    // Textures
    TextureAsset* spyBalloonTexture =
        TextureAsset::create(new TextureAsset("spy_balloon", "assets/textures/spy-balloon.png"));

    gGrassTileTexture =
        TextureAsset::create(new TextureAsset("grass_tile", "assets/textures/grass-tile.png"));

    TextureAsset* explosionTexture =
        TextureAsset::create(new TextureAsset("explosion", "assets/textures/explosion.png"));
    explosionTexture->type = TextureType::ATLAS;
    explosionTexture->spriteSheetDimensions = {4, 4};
    explosionTexture->useDefaultAnimation();

    TextureAsset* groundExplosionTexture = TextureAsset::create(
        new TextureAsset("ground_explosion", "assets/textures/ground-explosion.png")
    );
    groundExplosionTexture->type = TextureType::ATLAS;
    groundExplosionTexture->spriteSheetDimensions = {1, 7};
    groundExplosionTexture->useDefaultAnimation();

    TextureAsset* fighterJetTexture =
        TextureAsset::create(new TextureAsset("fighter_jet", "assets/textures/fighter-jet.png"));
    fighterJetTexture->type = TextureType::ATLAS;
    fighterJetTexture->spriteSheetDimensions = {1, 4};
    fighterJetTexture->useDefaultAnimation();

    TextureAsset* missileTexture =
        TextureAsset::create(new TextureAsset("missile", "assets/textures/missile.png"));
    missileTexture->type = TextureType::ATLAS;
    missileTexture->spriteSheetDimensions = {1, 3};
    missileTexture->useDefaultAnimation();

    TextureAsset::create(new TextureAsset("aa_gun", "assets/textures/aa-gun.png"));
    TextureAsset::create(new TextureAsset("shell", "assets/textures/shell.png"));
    TextureAsset::create(new TextureAsset("force_field", "assets/textures/force-field.png"));

    // Entities
    gSpyBalloon = Entity::create(new SpyBalloon);
    gSpyBalloon->scale = {50.0f, 50.0f};
    gSpyBalloon->colliderDimensions = Vector2Scale(gSpyBalloon->scale, 1.0f);
    gSpyBalloon->acceleration.y = constants::G_ACCEL;

    gMilitary = new Military(gSpyBalloon);

    Vector2 tileSize = {50.0f, 50.0f};
    uint32_t tileCount = 2 * constants::SCREEN_WIDTH / tileSize.x;

    float leftMostX = constants::SCREEN_CENTER.x - (tileCount * tileSize.x) / 2.0f;

    for (uint32_t i = 0; i < tileCount; ++i) {
        auto tile = Entity::create(new PhysicsEntity(gGrassTileTexture));
        tile->scale = tileSize;
        tile->colliderDimensions = tile->scale;
        tile->position = {leftMostX + i * tileSize.x, constants::SCREEN_HEIGHT - tileSize.y / 2.0f};
        tile->anchored = true;

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

    gHealthBar.position = {125.0f, 50.0f};
    gHealthBar.backgroundColor = BLACK;
    gHealthBar.barColor = RED;
    gHealthBar.textColor = WHITE;
    gHealthBar.size = {200.0f, 40.0f};
    gHealthBar.title = "HP";

    gFuelBar.position = {125.0f, 100.0f};
    gFuelBar.backgroundColor = BLACK;
    gFuelBar.barColor = BROWN;
    gFuelBar.textColor = WHITE;
    gFuelBar.size = {200.0f, 40.0f};
    gFuelBar.title = "Fuel";

    gForceFieldStatus.position = {300.0f, 80.0f};
    gForceFieldStatus.backgroundColor = GOLD;
    gForceFieldStatus.size = {80.0f, 80.0f};
    gForceFieldStatus.texture = TextureAsset::get("force_field");
    gForceFieldStatus.key = KeyboardKey::KEY_F;

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

            if (IsKeyDown(KeyboardKey::KEY_F)) {
                gSpyBalloon->onInput(KeyboardKey::KEY_F);
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
    if (!gSpyBalloon->isAlive() || gSpyBalloon->landedSuccessfully) {
        gGameStatus = GameStatus::ENDED;
    }

    gMilitary->update(deltaTime);

    // Set collision candidates
    for (auto& pair : Entity::registry()) {
        Entity* ptr = pair.second.get();
        if (auto mv = dynamic_cast<MilitaryVehicle*>(ptr)) {
            mv->setCollisionCandidates(gMilitary->vehicleCollisionCandidates);
        } else if (auto missile = dynamic_cast<Missile*>(ptr)) {
            missile->setCollisionCandidates(gMilitary->projectileCollisionCandidates);
        }
    }

    for (auto& pair : Entity::registry()) {
        pair.second->update(deltaTime);
    }

    gSpyBalloon->setCollisionCandidates(gMapTiles);
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

            drawText(
                "Save the Chinese Spy Balloon",
                {constants::SCREEN_CENTER.x, constants::SCREEN_CENTER.y - 50},
                constants::ui::HEADER_FONT_SIZE,
                WHITE
            );

            break;
        }
        case (GameStatus::STARTING): {
            renderTintedOverlay();
            drawText(
                "Land our balloon by any means necessary.",
                {constants::SCREEN_CENTER.x, constants::SCREEN_CENTER.y - 100},
                constants::ui::HEADER_FONT_SIZE,
                WHITE
            );

            drawText(
                TextFormat("Starting in %i...", static_cast<int32_t>(std::ceil(gRoundCountdown))),
                constants::SCREEN_CENTER,
                constants::ui::HEADER_FONT_SIZE,
                WHITE
            );

            break;
        }
        case (GameStatus::IN_PROGRESS): {
            gHealthBar.render(gSpyBalloon->health, gSpyBalloon->maxHealth);
            gFuelBar.render(gSpyBalloon->fuel, gSpyBalloon->maxFuel);
            gForceFieldStatus.render(
                gSpyBalloon->lastForceFieldUseTime,
                gSpyBalloon->forceFieldCooldown
            );

            break;
        }
        case (GameStatus::ENDED): {
            renderTintedOverlay();

            if (gSpyBalloon->isAlive()) {
                drawText(
                    "Hooray! You saved the spy balloon!",
                    constants::SCREEN_CENTER,
                    constants::ui::HEADER_FONT_SIZE,
                    WHITE
                );
            } else {
                drawText(
                    "Oh no, they blew us up!",
                    constants::SCREEN_CENTER,
                    constants::ui::HEADER_FONT_SIZE,
                    WHITE
                );
            }

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
    delete gMilitary;

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