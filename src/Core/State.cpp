#include "State.h"
#include "../UI/UICreationEngine.h"
#include "Application.h"
#include "Logger.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

void State::ApplyStateConfig(const std::string& stateName) {
    #ifndef C3D_RUNTIME
    std::string statesJsonPath = (std::filesystem::path(Application::Get().GetProjectRoot()) / "states.json").string();
    #else
    std::string statesJsonPath = "states.json";
    #endif

    if (!std::filesystem::exists(statesJsonPath)) return;
    
    std::ifstream file(statesJsonPath);
    if (!file.is_open()) return;

    try {
        nlohmann::json statesJson = nlohmann::json::parse(file);
        if (statesJson.contains(stateName)) {
            auto& stateData = statesJson[stateName];
            
            if (stateData.contains("camera")) {
                auto& camData = stateData["camera"];
                Camera* cam = Application::Get().GetCamera();
                if (cam) {
                    if (camData.contains("position")) {
                        cam->Position = glm::vec3(camData["position"][0], camData["position"][1], camData["position"][2]);
                    }
                    if (camData.contains("orientation")) {
                        cam->Orientation = glm::vec3(camData["orientation"][0], camData["orientation"][1], camData["orientation"][2]);
                    }
                    if (camData.contains("yaw")) cam->yaw = camData["yaw"];
                    if (camData.contains("pitch")) cam->pitch = camData["pitch"];
                    if (camData.contains("fov")) cam->FOV = camData["fov"];
                }
            }

            if (stateData.contains("activeObjects")) {
                auto& activeObjNames = stateData["activeObjects"];
                std::vector<std::string> activeList;
                for (auto& name : activeObjNames) {
                    activeList.push_back(name.get<std::string>());
                }

                auto scene = Application::Get().GetScene();
                if (scene) {
                    for (auto& obj : scene->GetObjects()) {
                        bool shouldBeActive = false;
                        for (const auto& activeName : activeList) {
                            if (obj.name == activeName) {
                                shouldBeActive = true;
                                break;
                            }
                        }
                        obj.isActive = shouldBeActive;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::AddLog("[State] Failed to parse states.json: %s", e.what());
    }
}

void State::LoadFromLayout(const std::string& screenName) {
    m_StateName = screenName;
    m_UIElements = UICreationEngine::GetElementsByScreen(screenName);
    ApplyStateConfig(screenName);
}
