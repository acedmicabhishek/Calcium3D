#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>


#include "Scene.h"
#include "Renderer.h"
#include "Camera.h"
#include "InputManager.h"
#include "RenderContext.h"
#include "RenderPipeline.h"
#include "GameState.h"
#include "../UI/Screens/Screen.h"
#include <map>

struct ApplicationSpecification {
    std::string Name = "Calcium3D Application";
    uint32_t Width = 1600;
    uint32_t Height = 900;
};

class Application {
public:
    Application(const ApplicationSpecification& spec);
    virtual ~Application();

    virtual void Run();
    void Close();
    virtual bool Init();

    Screen* GetActiveScreen() const { return m_ActiveScreen; }
    
    static Application& Get() { return *s_Instance; }
    static Application* GetInstance() { return s_Instance; }
    GLFWwindow* GetWindow() const { return m_Window; }
    
    GameState m_StartGameState = GameState::START_SCREEN;

    bool& GetShowSkybox() { return m_ShowSkybox; }

    virtual void OpenProject(const std::string& path);
    virtual void CreateProject(const std::string& path);
    const std::string& GetProjectRoot() const { return m_ProjectRoot; }
    std::string GetProjectName() const;

public:
    void AddScreen(int stateId, std::unique_ptr<Screen> screen);
    Scene* GetScene() { return m_Scene.get(); }
    Camera* GetCamera() { return m_Camera.get(); }

protected:
    virtual void Shutdown();
    void ChangeState(int newState);
    
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRender() = 0;
    virtual void PostRender() {}

    ApplicationSpecification m_Specification;
    GLFWwindow* m_Window;
    bool m_Running = true;
    bool m_Initialized = false;
    std::string m_ProjectRoot = "";
    
    std::unique_ptr<Scene> m_Scene;

    std::unique_ptr<RenderPipeline> m_RenderPipeline;
    RenderContext m_RenderContext;
    
    std::unique_ptr<Camera> m_Camera;

    std::unique_ptr<class Water> m_Water;
    std::unique_ptr<class Cloud2D> m_Cloud2D;
    std::unique_ptr<class VolumetricCloud> m_VolumetricCloud;
    
    
    std::map<int, std::unique_ptr<Screen>> m_Screens; 
    Screen* m_ActiveScreen = nullptr;
    
    bool m_ShowSkybox = true;
    bool m_ShowGradientSky = false;
    bool m_ShowWater = false; 
    bool m_ShowClouds = false; 
    
    
    int m_MSAASamples = 0; 
    unsigned int m_MSAAFBO = 0, m_MSAAColorBuffer = 0, m_MSAARBO = 0;
    void CreateMSAAFramebuffer(int samples);
    void ResizeMSAAFramebuffer(int width, int height);

    
    unsigned int m_ViewportFBO = 0;
    unsigned int m_ViewportTexture = 0;
    unsigned int m_ViewportRBO = 0;
    int m_ViewportWidth = 800;
    int m_ViewportHeight = 600;
    void CreateViewportFramebuffer(int width, int height);

    static Application* s_Instance;
};

#endif
