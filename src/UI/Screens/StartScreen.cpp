#include "StartScreen.h"
#include "../UIManager.h"
#include "../../Core/GameState.h"
#include <iostream>

void StartScreen::Init() {
    LoadFromLayout("StartScreen");

    
    for (auto& el : m_UIElements) {
        if (el.name == "StartBtn") {
            el.onClick = []() { GameStateManager::SetState(GameState::GAMEPLAY); };
        } else if (el.name == "ExitBtn") {
            el.onClick = []() { GameStateManager::SetState(GameState::EXIT); };
        }
    }
}

void StartScreen::Update(float deltaTime) {
}

void StartScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    UIManager::Render(m_UIElements, canvasSize, baseScreenPos);
}
