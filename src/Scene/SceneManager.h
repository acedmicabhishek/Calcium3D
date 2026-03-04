#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "Scene.h"
#include "TransitionType.h"
#include <string>
#include <memory>
#include <functional>
#include "../Renderer/Camera.h"

class SceneManager {
public:
    static SceneManager& Get() {
        static SceneManager instance;
        return instance;
    }

    void SetActiveScene(Scene* scene) { m_ActiveScene = scene; }
    Scene* GetActiveScene() { return m_ActiveScene; }
    
    void SetMainCamera(Camera* camera) { m_MainCamera = camera; }
    Camera* GetMainCamera() { return m_MainCamera; }

    void LoadScene(const std::string& path);
    void TransitionToScene(const std::string& path, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void TransitionToFlag(const std::string& flagName, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void JumpToFlag(const std::string& flagName);
    
    void Update(float dt);
    void RenderUI(); 

    bool IsTransitioning() const { return m_InTransition; }
    float GetTransitionProgress() const { return m_TransitionProgress; }
    TransitionType GetTransitionType() const { return m_TransitionType; }

private:
    SceneManager() = default;
    
    Scene* m_ActiveScene = nullptr;
    
    bool m_InTransition = false;
    float m_TransitionProgress = 0.0f;
    float m_TransitionDuration = 1.0f;
    TransitionType m_TransitionType = TransitionType::None;
    std::string m_PendingScenePath = "";
    std::string m_PendingFlag = "";
    Camera* m_MainCamera = nullptr;
    
    
    bool m_TeleportPending = false;
    Scene::FlagData m_TeleportTarget;
    
    void CompleteTransition();
    void ApplyTeleport();
};

#endif
