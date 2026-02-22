#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <glm/glm.hpp>
#include <string>

struct GameObject; 

class Behavior {
public:
    virtual ~Behavior() = default;

    virtual void OnStart() {}
    virtual void OnUpdate(float dt) {}

    GameObject* gameObject = nullptr;
    bool enabled = true;
};

#endif
