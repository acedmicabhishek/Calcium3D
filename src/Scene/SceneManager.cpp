#include "SceneManager.h"
#include "../Core/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void SceneManager::LoadScene(const std::string& path) {
    if (!m_ActiveScene) {
        Logger::AddLog("[SceneManager] [ERROR] No active scene set to load into!");
        return;
    }
    
    Logger::AddLog("[SceneManager] Loading scene: %s", path.c_str());
    m_ActiveScene->Load(path);
}

void SceneManager::TransitionToScene(const std::string& path, TransitionType type, float duration) {
    if (m_InTransition) return;
    
    m_InTransition = true;
    m_TransitionType = type;
    m_TransitionDuration = duration;
    m_TransitionProgress = 0.0f;
    m_PendingScenePath = path;
    
    Logger::AddLog("[SceneManager] Starting transition to: %s", path.c_str());
}

void SceneManager::TransitionToFlag(const std::string& flagName, TransitionType type, float duration) {
    if (m_InTransition) return;

    
    if (type == TransitionType::None) {
        JumpToFlag(flagName);
        return;
    }

    m_InTransition = true;
    m_TransitionType = type;
    m_TransitionDuration = (duration > 0.0f ? duration : 1.0f);
    m_TransitionProgress = 0.0f;
    m_PendingFlag = flagName;
    m_PendingScenePath = "";

    Logger::AddLog("[SceneManager] Starting transition to flag: %s", flagName.c_str());
}

void SceneManager::Update(float dt) {
    if (m_MainCamera && m_ActiveScene && m_MainCamera->parentIndex != -1) {
        if (m_MainCamera->parentIndex < (int)m_ActiveScene->GetObjects().size()) {
            glm::mat4 globalT = m_ActiveScene->GetGlobalTransform(m_MainCamera->parentIndex);
            m_MainCamera->Position = glm::vec3(globalT[3]);
        } else {
            m_MainCamera->parentIndex = -1;
        }
    }
    
    if (m_TeleportPending) {
        ApplyTeleport();
    }
    
    if (!m_InTransition) return;
    
    m_TransitionProgress += dt / m_TransitionDuration;
    
    
    if (m_TransitionProgress >= 0.5f) {
        if (!m_PendingScenePath.empty()) {
            LoadScene(m_PendingScenePath);
            m_PendingScenePath = "";
        }
        else if (!m_PendingFlag.empty()) {
            JumpToFlag(m_PendingFlag);
            m_PendingFlag = "";
        }
    }
    
    if (m_TransitionProgress >= 1.0f) {
        CompleteTransition();
    }
}

void SceneManager::CompleteTransition() {
    m_InTransition = false;
    m_TransitionProgress = 1.0f;
    m_TransitionType = TransitionType::None;
    Logger::AddLog("[SceneManager] Transition completed.");
}

void SceneManager::JumpToFlag(const std::string& flagName) {
    if (!m_ActiveScene || !m_MainCamera) {
        Logger::AddLog("[SceneManager] JumpToFlag failed: no scene or camera! scene=%p cam=%p", m_ActiveScene, m_MainCamera);
        return;
    }
    
    auto const& flags = m_ActiveScene->GetFlags();
    auto it = flags.find(flagName);
    if (it == flags.end()) {
        Logger::AddLog("[SceneManager] JumpToFlag: flag '%s' not found!", flagName.c_str());
        return;
    }
    
    m_TeleportTarget = it->second;
    m_TeleportPending = true;
    Logger::AddLog("[SceneManager] Teleport queued for flag: '%s' -> (%.1f, %.1f, %.1f) yaw=%.1f pitch=%.1f", 
        flagName.c_str(), m_TeleportTarget.position.x, m_TeleportTarget.position.y, m_TeleportTarget.position.z,
        m_TeleportTarget.yaw, m_TeleportTarget.pitch);
}

void SceneManager::ApplyTeleport() {
    if (!m_MainCamera) return;
    m_MainCamera->Position = m_TeleportTarget.position;
    m_MainCamera->yaw     = m_TeleportTarget.yaw;
    m_MainCamera->pitch   = m_TeleportTarget.pitch;
    
    glm::vec3 dir;
    dir.x = cos(glm::radians(m_MainCamera->yaw)) * cos(glm::radians(m_MainCamera->pitch));
    dir.y = sin(glm::radians(m_MainCamera->pitch));
    dir.z = sin(glm::radians(m_MainCamera->yaw)) * cos(glm::radians(m_MainCamera->pitch));
    m_MainCamera->Orientation = glm::normalize(dir);
    m_TeleportPending = false;
    Logger::AddLog("[SceneManager] Teleport applied: pos=(%.1f,%.1f,%.1f) yaw=%.1f pitch=%.1f",
        m_TeleportTarget.position.x, m_TeleportTarget.position.y, m_TeleportTarget.position.z,
        m_TeleportTarget.yaw, m_TeleportTarget.pitch);
}

void SceneManager::RenderUI() {
    
}
