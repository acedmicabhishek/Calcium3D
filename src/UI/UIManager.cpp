#include "UIManager.h"
#include <imgui.h>
#include "../Core/StateManager.h"
#include "../Core/InputManager.h"
#include "../Core/Application.h"
#include "../AudioEngine/AudioEngine.h"
#include "../Scene/SceneManager.h"

void UIManager::Render(const std::vector<UIElement>& elements, glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    for (const auto& el : elements) {
        RenderElement(el, canvasSize, baseScreenPos);
    }
}

void UIManager::RenderElement(const UIElement& element, glm::vec2 canvasSize, glm::vec2 baseScreenPos) {
    
    float centerX = canvasSize.x * element.anchorMin.x;
    float centerY = canvasSize.y * element.anchorMin.y;
    
    
    float finalX = centerX + element.position.x - (element.size.x * element.pivot.x);
    float finalY = centerY + element.position.y - (element.size.y * element.pivot.y);

    
    ImGui::SetCursorScreenPos(ImVec2(baseScreenPos.x + finalX, baseScreenPos.y + finalY));

    ImGui::PushID(element.name.c_str());

    
    if (element.type == UIElementType::TEXT) {
        ImGui::TextColored(ImVec4(element.color.r, element.color.g, element.color.b, element.color.a), "%s", element.text.c_str());
    } else if (element.type == UIElementType::BUTTON) {
        if (ImGui::Button(element.text.c_str(), ImVec2(element.size.x, element.size.y))) {
            if (element.actionType == "PlayAudio") {
                Logger::AddLog("[UI] Button Click: %s -> Script: PlayAudio, Target: %s", 
                               element.name.c_str(), element.targetAudioObject.c_str());
            } else if (element.actionType == "PlayVideo" || element.actionType == "PauseVideo" || element.actionType == "ToggleVideo") {
                Logger::AddLog("[UI] Button Click: %s -> Script: %s, Target: %s", 
                               element.name.c_str(), element.actionType.c_str(), 
                               element.targetVideoObject.empty() ? "(First Video Fallback)" : element.targetVideoObject.c_str());
            } else {
                Logger::AddLog("[UI] Button Click: %s -> Script: %s, Target: %s", 
                               element.name.c_str(), element.actionType.c_str(), element.targetState.c_str());
            }
            
            InputManager::RegisterUIButtonClick(element.name);
            
            if (element.onClick) element.onClick();
            
            if (element.actionType == "ChangeState" && !element.targetState.empty()) {
                StateManager::ChangeState(element.targetState);
            } else if (element.actionType == "PushState" && !element.targetState.empty()) {
                StateManager::PushState(element.targetState);
            } else if (element.actionType == "PopState") {
                StateManager::PopState();
            } else if (element.actionType == "TransitionToFlag" && !element.targetFlag.empty()) {
                SceneManager::Get().TransitionToFlag(element.targetFlag, (TransitionType)element.transitionType, element.transitionDuration);
            } else if (element.actionType == "PlayAudio" && !element.targetAudioObject.empty()) {
                if (Scene* scene = Application::Get().GetScene()) {
                    for (auto& obj : scene->GetObjects()) {
                        if (obj.name == element.targetAudioObject && obj.hasAudio) {
                            if (!obj.audio.playing) {
                                obj.audio.playing = true;
                                AudioEngine::PlayObjectAudio(obj);
                                Logger::AddLog("[UI] Playing audio for target: %s", obj.name.c_str());
                            } else {
                                obj.audio.playing = false;
                                AudioEngine::StopObjectAudio(obj);
                                Logger::AddLog("[UI] Stopping audio for target: %s", obj.name.c_str());
                            }
                            break;
                        }
                    }
                }
            } else if (element.actionType == "PlayVideo" || element.actionType == "PauseVideo" || element.actionType == "ToggleVideo") {
                if (Scene* scene = Application::Get().GetScene()) {
                    GameObject* target = nullptr;
                    
                    
                    if (!element.targetVideoObject.empty()) {
                        for (auto& obj : scene->GetObjects()) {
                            if (obj.name == element.targetVideoObject && obj.hasScreen && obj.screen.type == ScreenType::Video) {
                                target = &obj;
                                break;
                            }
                        }
                    }
                    
                    
                    if (!target) {
                        for (auto& obj : scene->GetObjects()) {
                            if (obj.hasScreen && obj.screen.type == ScreenType::Video) {
                                target = &obj;
                                Logger::AddLog("[UI] Target empty or not found. Falling back to first video: %s", target->name.c_str());
                                break;
                            }
                        }
                    }

                    if (target) {
                        if (element.actionType == "PlayVideo") target->screen.videoPaused = false;
                        else if (element.actionType == "PauseVideo") target->screen.videoPaused = true;
                        else if (element.actionType == "ToggleVideo") target->screen.videoPaused = !target->screen.videoPaused;
                        
                        Logger::AddLog("[UI] %s video for target: %s", (target->screen.videoPaused ? "Pausing" : "Playing"), target->name.c_str());
                    } else {
                        Logger::AddLog("[UI] WARNING: No video objects found in scene to %s!", element.actionType.c_str());
                    }
                }
            }
        }
    } else if (element.type == UIElementType::CHECKBOX) {
        bool val = false;
        ImGui::Checkbox(element.text.c_str(), &val);
    } else if (element.type == UIElementType::SLIDER) {
        float fval = 0.5f;
        ImGui::SliderFloat(element.text.c_str(), &fval, 0.0f, 1.0f);
    }
    
    ImGui::PopID();
}
