#include "PlayMode.h"
#include <imgui.h>
#include "../Core/Logger.h"
#include "../Core/InputManager.h"
#include "../Core/Application.h"
#include "../Core/StateManager.h"
#include "../AudioEngine/AudioEngine.h"
#include "../Scene/Scene.h"

void PlayMode::RenderAndHandleClicks(
    std::vector<UIElement>& elements,
    float viewportWidth, float viewportHeight,
    ImVec2 cursorPos,
    std::string& activeScreen
) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 mousePos = ImGui::GetMousePos();
    bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    
    for (auto& el : elements) {
        float centerX = viewportWidth * el.anchorMin.x;
        float centerY = viewportHeight * el.anchorMin.y;
        float finalX = centerX + el.position.x - (el.size.x * el.pivot.x);
        float finalY = centerY + el.position.y - (el.size.y * el.pivot.y);
        
        float screenX = cursorPos.x + finalX;
        float screenY = cursorPos.y + finalY;
        float screenX2 = screenX + el.size.x;
        float screenY2 = screenY + el.size.y;
        
        
        bool hovered = (mousePos.x >= screenX && mousePos.x <= screenX2 &&
                        mousePos.y >= screenY && mousePos.y <= screenY2);
        
        if (el.type == UIElementType::BUTTON) {
            
            ImU32 bgColor = hovered ? ImColor(0.35f, 0.35f, 0.45f, 0.9f) : ImColor(0.22f, 0.22f, 0.30f, 0.85f);
            ImU32 borderColor = hovered ? ImColor(0.6f, 0.7f, 1.0f, 1.0f) : ImColor(0.4f, 0.4f, 0.5f, 0.8f);
            drawList->AddRectFilled(ImVec2(screenX, screenY), ImVec2(screenX2, screenY2), bgColor, 4.0f);
            drawList->AddRect(ImVec2(screenX, screenY), ImVec2(screenX2, screenY2), borderColor, 4.0f);
            
            
            ImVec2 textSize = ImGui::CalcTextSize(el.text.c_str());
            float textX = screenX + (el.size.x - textSize.x) * 0.5f;
            float textY = screenY + (el.size.y - textSize.y) * 0.5f;
            drawList->AddText(ImVec2(textX, textY), ImColor(el.color.r, el.color.g, el.color.b, el.color.a), el.text.c_str());
            
            
            if (hovered && mouseClicked) {
                Logger::AddLog("[UI] Play Mode Click: %s -> %s", el.name.c_str(), el.actionType.c_str());
                InputManager::RegisterUIButtonClick(el.name);
                if (el.onClick) el.onClick();
                
                if (el.actionType == "ChangeState" && !el.targetState.empty()) {
                    activeScreen = el.targetState;
                    Logger::AddLog("[UI] Play Mode: Switching to screen '%s'", el.targetState.c_str());
                } else if (el.actionType == "PushState" && !el.targetState.empty()) {
                    activeScreen = el.targetState;
                } else if (el.actionType == "PopState") {
                    
                } else if (el.actionType == "PlayAudio" && !el.targetAudioObject.empty()) {
                    if (Scene* sc = Application::Get().GetScene()) {
                        for (auto& obj : sc->GetObjects()) {
                            if (obj.name == el.targetAudioObject && obj.hasAudio) {
                                if (!obj.audio.playing) {
                                    obj.audio.playing = true;
                                    AudioEngine::PlayObjectAudio(obj);
                                } else {
                                    obj.audio.playing = false;
                                    AudioEngine::StopObjectAudio(obj);
                                }
                                break;
                            }
                        }
                    }
                } else if (el.actionType == "PlayVideo" || el.actionType == "PauseVideo" || el.actionType == "ToggleVideo") {
                    if (Scene* sc = Application::Get().GetScene()) {
                        GameObject* target = nullptr;
                        if (!el.targetVideoObject.empty()) {
                            for (auto& obj : sc->GetObjects()) {
                                if (obj.name == el.targetVideoObject && obj.hasScreen && obj.screen.type == ScreenType::Video) {
                                    target = &obj;
                                    break;
                                }
                            }
                        }
                        if (!target) {
                            for (auto& obj : sc->GetObjects()) {
                                if (obj.hasScreen && obj.screen.type == ScreenType::Video) {
                                    target = &obj;
                                    break;
                                }
                            }
                        }
                        if (target) {
                            if (el.actionType == "PlayVideo") target->screen.videoPaused = false;
                            else if (el.actionType == "PauseVideo") target->screen.videoPaused = true;
                            else if (el.actionType == "ToggleVideo") target->screen.videoPaused = !target->screen.videoPaused;
                            Logger::AddLog("[UI] %s video: %s", el.actionType.c_str(), target->name.c_str());
                        }
                    }
                }
            }
        } else if (el.type == UIElementType::TEXT) {
            
            drawList->AddText(ImVec2(screenX, screenY), 
                ImColor(el.color.r, el.color.g, el.color.b, el.color.a), el.text.c_str());
        }
    }
}
