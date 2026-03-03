#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <glm/glm.hpp>

class Camera;
struct RenderContext;

class Console {
public:
    Console();
    ~Console();

    void Init();
    void Toggle();
    void Render(Camera* camera, RenderContext& ctx);

    bool IsOpen() const { return m_Open; }
    void AddEngineLog(const std::string& msg);

    
    void SetSkyboxEnabled(bool v) { m_ShowSkybox = v; }
    bool IsSkyboxEnabled() const { return m_ShowSkybox; }

    void SetGradientSkyEnabled(bool v) { m_ShowGradientSky = v; }
    bool IsGradientSkyEnabled() const { return m_ShowGradientSky; }

    void SetCloudsEnabled(bool v) { m_ShowClouds = v; }
    bool IsCloudsEnabled() const { return m_ShowClouds; }

    
    bool IsTimePaused() const { return m_TimePaused; }
    float GetTimeSpeed() const { return m_TimeSpeed; }

private:
    void ExecuteCommand(const std::string& cmd);
    void AddLog(const char* fmt, ...);

    
    void RenderCameraSettings(Camera* camera);
    void RenderRenderingSettings(RenderContext& ctx);
    void RenderPhysicsSettings();
    void RenderEnvironmentSettings(RenderContext& ctx);
    void RenderLoggingScreen();

    bool m_Open = false;
    bool m_EngineLoggingEnabled = false;

    
    std::vector<std::string> m_Log;
    char m_InputBuf[256] = {};
    bool m_ScrollToBottom = false;
    bool m_ReclaimFocus = false;

    
    enum class SettingsPanel { None, Camera, Rendering, Physics, Environment, Logging };
    SettingsPanel m_ActivePanel = SettingsPanel::None;

    
    bool m_ShowSkybox = true;
    bool m_ShowGradientSky = false;
    bool m_ShowClouds = false;
    bool m_TimePaused = false;
    float m_TimeSpeed = 1.0f;

    
    bool m_Wireframe = false;
    bool m_VSync = true;
};

#endif
