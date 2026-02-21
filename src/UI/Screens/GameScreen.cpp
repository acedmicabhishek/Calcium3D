#include "GameScreen.h"
#include "../UIManager.h"
#include "../../Core/GameState.h"

void GameScreen::Init() {
    LoadFromLayout("GameScreen");

    for (auto& el : m_UIElements) {
        if (el.name == "BackToMenu") {
            el.onClick = []() { GameStateManager::SetState(GameState::START_SCREEN); };
        }
    }
}

void GameScreen::Update(float deltaTime) {
}

void GameScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    UIManager::Render(m_UIElements, canvasSize, baseScreenPos);
}
