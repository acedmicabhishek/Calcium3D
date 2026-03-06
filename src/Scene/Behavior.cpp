#include "Behavior.h"
#include "SceneManager.h"
#include "Scene.h"
#include "BehaviorRegistry.h"
#include "Builtin/SceneTransitionBehavior.h"
#include "../Core/Logger.h"
#include "../Renderer/Camera.h"

REGISTER_BEHAVIOR(SceneTransitionBehavior)


Camera* Behavior::GetCamera() const {
    return SceneManager::Get().GetMainCamera();
}

Scene* Behavior::GetScene() const {
    return SceneManager::Get().GetActiveScene();
}


glm::vec3 Behavior::GetPosition() const {
    return gameObject ? gameObject->position : glm::vec3(0.0f);
}

void Behavior::SetPosition(const glm::vec3& pos) {
    if (gameObject) gameObject->position = pos;
}

void Behavior::SetPosition(float x, float y, float z) {
    if (gameObject) gameObject->position = glm::vec3(x, y, z);
}

void Behavior::SetScale(const glm::vec3& scale) {
    if (gameObject) gameObject->scale = scale;
}

void Behavior::SetScale(float x, float y, float z) {
    if (gameObject) gameObject->scale = glm::vec3(x, y, z);
}


void Behavior::LoadScene(const std::string& path) {
    SceneManager::Get().LoadScene(path);
}

void Behavior::TransitionToScene(const std::string& path, TransitionType type, float duration) {
    SceneManager::Get().TransitionToScene(path, type, duration);
}

void Behavior::TransitionToFlag(const std::string& flagName, TransitionType type, float duration) {
    SceneManager::Get().TransitionToFlag(flagName, type, duration);
}

void Behavior::JumpToFlag(const std::string& flagName) {
    SceneManager::Get().JumpToFlag(flagName);
}


void Behavior::SetProperty(const std::string& name, const std::any& value) {
    Scene* scene = GetScene();
    if (scene) scene->SetProperty(name, value);
}

std::any Behavior::GetProperty(const std::string& name) const {
    Scene* scene = GetScene();
    if (scene) return scene->GetProperty(name);
    return std::any();
}
