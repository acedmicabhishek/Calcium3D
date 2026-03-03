#ifndef EDITOR_APPLICATION_H
#define EDITOR_APPLICATION_H

#include "Application.h"
#include "EditorLayer.h"
#include "Home.h"
#include "Gizmo.h"

class EditorApplication : public Application {
public:
    EditorApplication(const ApplicationSpecification& spec);
    ~EditorApplication() override;

    enum class AppState { Home, Editor };
    void SetState(AppState state) { m_State = state; }
    AppState GetState() const { return m_State; }

    void OpenProject(const std::string& path) override;
    void CreateProject(const std::string& path) override;
    void SaveProject(bool silent = false);
    void SaveGlobalSettings();
    void LoadGlobalSettings();
    bool Init() override;

    bool IsPlayMode() const { return m_PlayMode; }
    void EnterPlayMode();
    void ExitPlayMode();
    void TogglePlayMode();

protected:
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void PostRender() override;
    void Shutdown() override;

private:
    void RenderEditor(float deltaTime);
    void RenderHome();

    
    int m_MSAASamples = 0; 
    unsigned int m_MSAAFBO = 0, m_MSAAColorBuffer = 0, m_MSAARBO = 0;
    void CreateMSAAFramebuffer(int samples);

    unsigned int m_ViewportFBO = 0;
    unsigned int m_ViewportTexture = 0;
    unsigned int m_ViewportRBO = 0;
    int m_ViewportWidth = 800;
    int m_ViewportHeight = 600;
    void CreateViewportFramebuffer(int width, int height);

    AppState m_State = AppState::Home;
    std::unique_ptr<EditorLayer> m_EditorLayer;
    std::unique_ptr<Home> m_Home;
    std::unique_ptr<Gizmo> m_Gizmo;

    bool m_PlayMode = false;
    std::string m_PlayModeSceneBackup; 
};

#endif
