#include "Application.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"
#include "Renderer.h"
#include "Editor.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include "Gizmo.h" 
#include <iostream>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"


struct WindowData {
    Camera* camera;
    EditorLayer* editor;
    Application* app;
};

Application* Application::s_Instance = nullptr;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (Application::GetInstance() && Application::GetInstance()->GetWindow() == window) {
         
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
        
    
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    
}

Application::Application(const ApplicationSpecification& spec)
    : m_Specification(spec)
{
    s_Instance = this;
    Init();
}

Application::~Application()
{
    Shutdown();
    s_Instance = nullptr;
}

void Application::Init()
{
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    
    m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSwapInterval(1); 

    
    InputManager::Init(m_Window);
    
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    
    m_Scene = std::make_unique<Scene>();
    
    
    m_EditorLayer = std::make_unique<EditorLayer>();
    m_EditorLayer->Init(m_Window);
    
    
    m_Camera = std::make_unique<Camera>(m_Specification.Width, m_Specification.Height, glm::vec3(0.0f, 0.0f, 2.0f));

    
    static WindowData windowData;
    windowData.camera = m_Camera.get();
    windowData.editor = m_EditorLayer.get();
    windowData.app = this;
    glfwSetWindowUserPointer(m_Window, &windowData);

    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    
    ResourceManager::LoadShader("default", "../shaders/default.vert", "../shaders/default.frag");
    ResourceManager::LoadShader("light", "../shaders/light.vert", "../shaders/light.frag");
    ResourceManager::LoadShader("skybox", "../shaders/skybox.vert", "../shaders/skybox.frag");
    ResourceManager::LoadShader("gradientSky", "../shaders/gradient_sky.vert", "../shaders/gradient_sky.frag");
    ResourceManager::LoadShader("water", "../shaders/water.vert", "../shaders/water.frag");
    ResourceManager::LoadShader("cloud2d", "../shaders/2dcloud.vert", "../shaders/2dcloud.frag");
    ResourceManager::LoadShader("volumetric_cloud", "../shaders/volumetric_cloud.vert", "../shaders/volumetric_cloud.frag");
    ResourceManager::LoadShader("gizmo", "../shaders/gizmo.vert", "../shaders/gizmo.frag");
    


    
    CreateViewportFramebuffer(m_Specification.Width, m_Specification.Height);
    
    
    m_Water = std::make_unique<Water>();
    m_Cloud2D = std::make_unique<Cloud2D>();
    m_VolumetricCloud = std::make_unique<VolumetricCloud>();
    m_Gizmo = std::make_unique<Gizmo>(); 
    
    
    m_RenderPipeline = std::make_unique<EditorPipeline>();
    m_RenderPipeline->Init(); 

    
    Texture defaultDiffuse = ResourceManager::LoadTexture("defaultDiffuse", "../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    Texture defaultSpecular = ResourceManager::LoadTexture("defaultSpecular", "../Resource/default/texture/DefaultTex.png", "specular", 1);

    

    
    
    
    
    
    Logger::AddLog("Application Initialized - Default Scene Loaded");
    m_Initialized = true;
}


void Application::CreateMSAAFramebuffer(int samples) {
    if (m_MSAAFBO != 0) {
        glDeleteFramebuffers(1, &m_MSAAFBO);
        glDeleteTextures(1, &m_MSAAColorBuffer);
        glDeleteRenderbuffers(1, &m_MSAARBO);
        m_MSAAFBO = 0;
        m_MSAAColorBuffer = 0;
        m_MSAARBO = 0;
    }

    m_MSAASamples = samples;
    if (m_MSAASamples > 0) {
        glGenFramebuffers(1, &m_MSAAFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);

        glGenTextures(1, &m_MSAAColorBuffer);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_RGB, m_ViewportWidth, m_ViewportHeight, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_MSAAColorBuffer, 0);

        glGenRenderbuffers(1, &m_MSAARBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples, GL_DEPTH24_STENCIL8, m_ViewportWidth, m_ViewportHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAARBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "MSAA Framebuffer error" << std::endl;
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Application::CreateViewportFramebuffer(int width, int height) {
    
    if (m_ViewportFBO != 0) {
        glDeleteFramebuffers(1, &m_ViewportFBO);
        glDeleteTextures(1, &m_ViewportTexture);
        glDeleteRenderbuffers(1, &m_ViewportRBO);
    }

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    
    glGenFramebuffers(1, &m_ViewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);

    
    glGenTextures(1, &m_ViewportTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ViewportTexture, 0);

    
    glGenRenderbuffers(1, &m_ViewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Viewport Framebuffer error!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::Run()
{
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(m_Window) && m_Running)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        
        if (!m_EditorLayer->isTimePaused) {
            m_EditorLayer->timeOfDay += deltaTime * m_EditorLayer->timeSpeed;
            if (m_EditorLayer->timeOfDay >= 24.0f) m_EditorLayer->timeOfDay -= 24.0f;
            if (m_EditorLayer->timeOfDay < 0.0f) m_EditorLayer->timeOfDay += 24.0f;
        }

        
        glfwPollEvents();
        InputManager::Update();

        
        if (Editor::isEditMode) {
             m_Camera->Inputs(m_Window);
        }

        
        m_ShowSkybox = m_EditorLayer->showSkybox;
        m_ShowGradientSky = m_EditorLayer->showGradientSky;
        m_ShowWater = m_EditorLayer->showWater;
        m_ShowClouds = m_EditorLayer->showClouds;

        
        if (m_MSAASamples != m_EditorLayer->msaaSamples) {
            CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
        }

        
        m_EditorLayer->Begin();

        if (m_EditorLayer->viewportWidth > 0 && m_EditorLayer->viewportHeight > 0 &&
            (m_EditorLayer->viewportWidth != m_ViewportWidth || m_EditorLayer->viewportHeight != m_ViewportHeight)) {
            CreateViewportFramebuffer(m_EditorLayer->viewportWidth, m_EditorLayer->viewportHeight);
            
            if (m_EditorLayer->msaaSamples > 0) {
                CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
            }
            
            m_Camera->width = m_ViewportWidth;
            m_Camera->height = m_ViewportHeight;
        }

        
        
        if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
        }
        glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 view = m_Camera->GetViewMatrix();
        glm::mat4 projection = m_Camera->GetProjectionMatrix();

        
        m_RenderContext.scene = m_Scene.get();
        m_RenderContext.camera = m_Camera.get();
        m_RenderContext.water = m_Water.get();
        m_RenderContext.cloud2d = m_Cloud2D.get();
        m_RenderContext.volCloud = m_VolumetricCloud.get();
        
        
        if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
            m_RenderContext.mainFBO = m_MSAAFBO;
        } else {
            m_RenderContext.mainFBO = m_ViewportFBO;
        }
        
        m_RenderContext.width = m_ViewportWidth;
        m_RenderContext.height = m_ViewportHeight;
        
        m_RenderContext.time = currentFrame;
        m_RenderContext.deltaTime = deltaTime;
        
        
        m_RenderContext.showWater = m_ShowWater;
        m_RenderContext.showClouds = m_ShowClouds;
        m_RenderContext.cloudMode = m_EditorLayer->cloudMode;
        m_RenderContext.waterHeight = m_EditorLayer->waterHeight;
        m_RenderContext.cloudHeight = m_EditorLayer->cloud2dHeight;
        
        m_RenderContext.cloudDensity = m_EditorLayer->cloudDensity; 
        m_RenderContext.cloudCover = m_EditorLayer->cloudCover;
        
        m_RenderContext.waveSpeed = m_EditorLayer->waveSpeed;
        m_RenderContext.waveStrength = m_EditorLayer->waveStrength;
        m_RenderContext.waterColor = m_EditorLayer->waterColor;

        m_RenderContext.showSkybox = m_ShowSkybox;
        m_RenderContext.showGradientSky = m_ShowGradientSky;

        
        float angle = (m_EditorLayer->timeOfDay - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
        glm::vec3 dynamicSunPos = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
        m_RenderContext.sunPosition = dynamicSunPos;
        m_RenderContext.sunColor = m_EditorLayer->sunColor;
        m_RenderContext.sunIntensity = m_EditorLayer->sunIntensity;
        m_RenderContext.sunEnabled = m_EditorLayer->sunEnabled;
        m_RenderContext.showSkybox = m_ShowSkybox;
        m_RenderContext.timeOfDay = m_EditorLayer->timeOfDay;
        m_RenderContext.sunBloom = m_EditorLayer->sunBloom;
        m_RenderContext.moonBloom = m_EditorLayer->moonBloom;
        m_RenderContext.globalTilingFactor = m_EditorLayer->globalTilingFactor;
        
        
        glm::vec3 dynamicMoonPos = glm::vec3(-dynamicSunPos.x, -dynamicSunPos.y, 0.0f);
        m_RenderContext.moonPosition = dynamicMoonPos;
        m_RenderContext.moonColor = m_EditorLayer->moonColor;
        m_RenderContext.moonIntensity = m_EditorLayer->moonIntensity;
        m_RenderContext.moonEnabled = m_EditorLayer->moonEnabled;
        
        
        m_RenderContext.msaaSamples = m_EditorLayer->msaaSamples;
        m_RenderContext.msaaSkyPass = m_EditorLayer->msaaSkyPass;
        m_RenderContext.msaaGeometryPass = m_EditorLayer->msaaGeometryPass;
        m_RenderContext.msaaTransparencyPass = m_EditorLayer->msaaTransparencyPass;
        
        
        m_RenderPipeline->Execute(m_RenderContext);

        
        if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ViewportFBO);
            glBlitFramebuffer(0, 0, m_ViewportWidth, m_ViewportHeight, 
                              0, 0, m_ViewportWidth, m_ViewportHeight, 
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        int winW, winH;
        glfwGetFramebufferSize(m_Window, &winW, &winH);
        glViewport(0, 0, winW, winH);
        glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        
        m_EditorLayer->viewportTextureID = m_ViewportTexture;

        
        

        
        m_EditorLayer->Render(*m_Scene, *m_Camera, deltaTime);
        m_EditorLayer->End();

        
        glfwSwapBuffers(m_Window);
    }
}

void Application::Close()
{
    m_Running = false;
}

void Application::Shutdown()
{
    if (!m_Initialized) return;
    
    m_EditorLayer->Shutdown();
    ResourceManager::Clear();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    
    m_Initialized = false;
}
