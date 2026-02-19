#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>


#include "Scene.h"
#include "Renderer.h"
#include "EditorLayer.h"
#include "Camera.h"
#include "InputManager.h"
#include "RenderContext.h"
#include "Pipelines/EditorPipeline.h"

struct ApplicationSpecification {
    std::string Name = "Calcium3D Application";
    uint32_t Width = 1600;
    uint32_t Height = 900;
};

class Application {
public:
    Application(const ApplicationSpecification& spec);
    ~Application();

    void Run();
    void Close();

    static Application& Get() { return *s_Instance; }
    static Application* GetInstance() { return s_Instance; }
    GLFWwindow* GetWindow() const { return m_Window; }

    bool& GetShowSkybox() { return m_ShowSkybox; }

private:
    void Init();
    void Shutdown();

    ApplicationSpecification m_Specification;
    GLFWwindow* m_Window;
    bool m_Running = true;
    bool m_Initialized = false;
    
    
    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<EditorLayer> m_EditorLayer;
    
    
    std::unique_ptr<EditorPipeline> m_RenderPipeline;
    RenderContext m_RenderContext;
    
    
    std::unique_ptr<Camera> m_Camera;

    std::unique_ptr<class Water> m_Water;
    std::unique_ptr<class Cloud2D> m_Cloud2D;
    std::unique_ptr<class VolumetricCloud> m_VolumetricCloud;
    std::unique_ptr<class Gizmo> m_Gizmo;
    
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
