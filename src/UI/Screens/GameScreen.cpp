#include "GameScreen.h"
#include "../UIManager.h"
#include "../../Core/GameState.h"

#include <imgui.h>
void GameScreen::Init() {
    LoadFromLayout(m_StateName);

    for (auto& el : m_UIElements) {
        if (el.name == "BackToMenu") {
            el.onClick = []() { GameStateManager::ChangeState(GameState::START_SCREEN); };
        }
    }
}

void GameScreen::Update(float deltaTime) {
}


void GameScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | 
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse;
                                   
    ImGui::SetNextWindowPos(ImVec2(baseScreenPos.x, baseScreenPos.y));
    ImGui::SetNextWindowSize(ImVec2(canvasSize.x, canvasSize.y));
    
    if (ImGui::Begin((m_StateName + "_UI").c_str(), nullptr, windowFlags)) {
        UIManager::Render(m_UIElements, canvasSize, baseScreenPos);
    }
    ImGui::End();
}
