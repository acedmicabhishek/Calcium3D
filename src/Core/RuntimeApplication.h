#ifndef RUNTIME_APPLICATION_H
#define RUNTIME_APPLICATION_H

#include "Application.h"
#include <nlohmann/json.hpp>

class RuntimeApplication : public Application {
public:
    RuntimeApplication(const ApplicationSpecification& spec);
    ~RuntimeApplication() override;
    bool Init() override;

protected:
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    
private:
    void LoadProjectConfig();
    float m_LastDeltaTime = 0.0f;
};

#endif
