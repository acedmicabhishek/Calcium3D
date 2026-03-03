#ifndef RUNTIME_APPLICATION_H
#define RUNTIME_APPLICATION_H

#include "Application.h"
#include "Console.h"
#include <nlohmann/json.hpp>
#include <memory>

class RuntimeApplication : public Application {
public:
    RuntimeApplication(const ApplicationSpecification& spec);
    ~RuntimeApplication() override;
    bool Init() override;

protected:
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void PostRender() override;
    
private:
    void LoadProjectConfig();
    void CreateDefaultScene();
    float m_LastDeltaTime = 0.0f;
    std::unique_ptr<Console> m_Console;
    bool m_ShowStateWarning = false;
    bool m_DisableStateWarning = false;
    bool m_IsDemoScene = false;
    float m_DemoTime = 0.0f;
    bool m_CursorCaptured = false;
    bool m_EscWasPressed = false;
};

#endif
