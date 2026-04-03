#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <string>
#include <any>
#include <glm/glm.hpp>
#include "TransitionType.h"
#include "../C3DprogrammingApi/C3D.h"

struct GameObject;
class Camera;
class Scene;

class Behavior {
public:
    virtual ~Behavior() = default;

    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnUI() {}

    virtual void OnCollisionEnter(GameObject* other) {}
    virtual void OnTriggerEnter(GameObject* other) {}

    GameObject* gameObject = nullptr;
    bool enabled = true;

    
    
    Camera* GetCamera() const;
    
    Scene*  GetScene() const;

    
    
    glm::vec3 GetPosition() const;
    void      SetPosition(const glm::vec3& pos);
    void      SetPosition(float x, float y, float z);

    void      SetScale(const glm::vec3& scale);
    void      SetScale(float x, float y, float z);

    
    void LoadScene(const std::string& path);
    void TransitionToScene(const std::string& path, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void TransitionToFlag(const std::string& flagName, TransitionType type = TransitionType::FadeBlack, float duration = 1.0f);
    void JumpToFlag(const std::string& flagName);

    
    void     SetProperty(const std::string& name, const std::any& value);
    std::any GetProperty(const std::string& name) const;
};

#endif
