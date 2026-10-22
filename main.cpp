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

float gRoundStartTime = 0.0f;
float gRoundElapsedTime = 0.0f;

Color gBackgroundColor = BLACK;

float gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

std::random_device randomDevice;
std::mt19937 generator(randomDevice());
std::uniform_real_distribution<float> puck_rng(0.0f, 1.0f);

TextButton gBackToMenuButton;
TextButton gPlayButton;
TextButton gQuitButton;

SpyBalloon* gSpyBalloon = nullptr;

void resetGame() {
    gRoundCountdown = constants::ROUND_COUNTDOWN_SEC;

    // Delete all pucks
}

void initialize() {
    InitWindow(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, "Save the Chinese Spy Balloon");

    SetTargetFPS(constants::TARGET_FPS);

    TextureAsset* spyBalloonTexture =
        TextureAsset::create(new TextureAsset("assets/textures/ai.png"));
    spyBalloonTexture->type = TextureType::SINGLE;

    gSpyBalloon = Entity::create(new SpyBalloon(spyBalloonTexture));
    gSpyBalloon->position = constants::SCREEN_CENTER;
    gSpyBalloon->scale = {250.0f, 250.0f};
    gSpyBalloon->colliderDimensions = Vector2Scale(gSpyBalloon->scale, 1.0f);

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
        gRoundStartTime = static_cast<float>(GetTime());
    }
}

void updateGameInProgress(float deltaTime) {
    gRoundElapsedTime = static_cast<float>(GetTime()) - gRoundStartTime;

    // Delete pucks over our limit

    for (auto& pair : Entity::registry()) {
        pair.second->update(deltaTime);
    }

    // Collisions
    for (auto& pair : Entity::registry()) {}

    if (gRoundElapsedTime >= constants::ROUND_MAX_TIME_SEC) {
        gGameStatus = GameStatus::ENDED;
    }
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
                updateGameStarting(deltaTime);
                break;
            }
            case (GameStatus::IN_PROGRESS): {
                updateGameInProgress(deltaTime);
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
            // Round timer
            drawText(
                TextFormat(
                    "%03i",
                    static_cast<int>(std::ceil(constants::ROUND_MAX_TIME_SEC - gRoundElapsedTime))
                ),
                {constants::SCREEN_CENTER.x, constants::ui::SCORE_Y_POS},
                constants::ui::HEADER_FONT_SIZE,
                WHITE
            );

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