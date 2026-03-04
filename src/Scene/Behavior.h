#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <string>
#include "TransitionType.h"

struct GameObject; 
class Behavior {
public:
    virtual ~Behavior() = default;

    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}

    GameObject* gameObject = nullptr;
    bool enabled = true;

    
    void LoadScene(const std::string& path);
    void TransitionToScene(const std::string& path, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void TransitionToFlag(const std::string& flagName, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void JumpToFlag(const std::string& flagName);
};

#endif
