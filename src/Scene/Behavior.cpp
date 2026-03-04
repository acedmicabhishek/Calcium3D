#include "Behavior.h"
#include "SceneManager.h"
#include "Scene.h"
#include "BehaviorRegistry.h"
#include "Builtin/SceneTransitionBehavior.h"
#include "../Core/Logger.h"

REGISTER_BEHAVIOR(SceneTransitionBehavior)

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
