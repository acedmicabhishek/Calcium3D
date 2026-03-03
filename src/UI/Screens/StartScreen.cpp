#include "StartScreen.h"
#include "../UIManager.h"
#include "../../Core/GameState.h"
#include <iostream>

void StartScreen::Init() {
    LoadFromLayout(m_StateName);
}

void StartScreen::Update(float deltaTime) {
}

#include <imgui.h>

void StartScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
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
