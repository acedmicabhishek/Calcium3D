#include "FallbackScreen.h"
#include "../../Core/GameState.h"
#include <imgui.h>
#include <string>

FallbackScreen::FallbackScreen() {
    m_StateId = GameStateManager::GetState(); 
}

void FallbackScreen::Init() {
    m_StateName = "FallbackScreen_" + std::to_string(m_StateId);
}

void FallbackScreen::Update(float deltaTime) {
}

void FallbackScreen::Render(glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    bool isCurrentState = true; 
    if (!isCurrentState) return;

    float panelW = 480.0f;
    float panelH = 320.0f;
    
    
    ImGui::SetNextWindowPos(ImVec2(baseScreenPos.x + 50, baseScreenPos.y + 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelW, 0), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.90f, 0.35f, 0.15f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.60f, 0.15f, 0.05f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.75f, 0.20f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.40f, 0.10f, 0.05f, 0.80f)); 
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.70f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 0.20f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 18));

    
    ImGui::Begin("Fallback Screen (Game State Not Configured)", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.20f, 1.0f));
    ImGui::SetWindowFontScale(1.3f);
    
    
    const char* warningText = "Game State Not Configured";
    float textW = ImGui::CalcTextSize(warningText).x;
    ImGui::SetCursorPosX((panelW - textW) * 0.5f);
    ImGui::Text("%s", warningText);
    
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.55f, 0.45f, 1.0f));
    
    const char* infoLine = "This is a fallback screen.";
    float infoW = ImGui::CalcTextSize(infoLine).x;
    ImGui::SetCursorPosX((panelW - infoW) * 0.5f);
    ImGui::Text("%s", infoLine);

    const char* infoLine2 = "The game state was not properly set up.";
    float info2W = ImGui::CalcTextSize(infoLine2).x;
    ImGui::SetCursorPosX((panelW - info2W) * 0.5f);
    ImGui::Text("%s", infoLine2);
    
    ImGui::PopStyleColor();

    ImGui::Spacing();

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.80f, 0.95f, 1.0f));
    
    std::string stateName = GameStateManager::GetStateName(m_StateId);
    std::string stateInfo = "Current State: " + stateName + " (ID: " + std::to_string(m_StateId) + ")";
    float stateW = ImGui::CalcTextSize(stateInfo.c_str()).x;
    ImGui::SetCursorPosX((panelW - stateW) * 0.5f);
    ImGui::Text("%s", stateInfo.c_str());
    
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.60f, 1.0f));
    
    const char* regTitle = "Registered States:";
    float regW = ImGui::CalcTextSize(regTitle).x;
    ImGui::SetCursorPosX((panelW - regW) * 0.5f);
    ImGui::Text("%s", regTitle);
    
    ImGui::PopStyleColor();

    auto& allStates = GameStateManager::GetAllStates();
    for (auto& [id, name] : allStates) {
        ImGui::PushStyleColor(ImGuiCol_Text, 
            (id == m_StateId) ? ImVec4(1.0f, 0.75f, 0.20f, 1.0f) : ImVec4(0.50f, 0.50f, 0.50f, 1.0f));
        
        std::string entry = (id == m_StateId ? ">> " : "   ") + 
                            std::to_string(id) + ": " + name +
                            (id == m_StateId ? " (current)" : "");
        float entryW = ImGui::CalcTextSize(entry.c_str()).x;
        ImGui::SetCursorPosX((panelW - entryW) * 0.5f);
        ImGui::Text("%s", entry.c_str());
        
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    
    float btnW = 200.0f;
    float btnH = 32.0f;
    float spacing = 10.0f;
    float totalW = btnW * 2 + spacing;
    ImGui::SetCursorPosX((panelW - totalW) * 0.5f);

    if (ImGui::Button("Next State  >>", ImVec2(btnW, btnH))) {
        
        int nextState = -1;
        bool foundCurrent = false;
        for (auto& [id, name] : allStates) {
            if (foundCurrent) {
                nextState = id;
                break;
            }
            if (id == GameStateManager::GetState()) {
                foundCurrent = true;
            }
        }
        
        if (nextState == -1 && !allStates.empty()) {
            nextState = allStates.begin()->first;
        }
        if (nextState >= 0) {
            GameStateManager::ChangeState(nextState);
        }
    }

    ImGui::SameLine(0, spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.40f, 0.65f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.50f, 0.80f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.30f, 0.55f, 1.0f));

    if (ImGui::Button("Go to Gameplay", ImVec2(btnW, btnH))) {
        GameStateManager::ChangeState(GameState::GAMEPLAY);
    }

    ImGui::PopStyleColor(3);

    ImGui::Spacing();

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.40f, 0.40f, 1.0f));
    const char* hint = "Set up screens in the Editor to remove this fallback.";
    float hintW = ImGui::CalcTextSize(hint).x;
    ImGui::SetCursorPosX((panelW - hintW) * 0.5f);
    ImGui::Text("%s", hint);
    ImGui::PopStyleColor();

    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(8); 
}
