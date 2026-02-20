#include "Application.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "ObjectFactory.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "Renderer.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include <iostream>
#include "../Physics/HitboxGraphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"
#include "Pipelines/StandardPipeline.h"

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
}

Application::~Application()
{
    Shutdown();
    s_Instance = nullptr;
}

bool Application::Init()
{
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    
    m_Window = glfwCreateWindow(m_Specification.Width, m_Specification.Height, m_Specification.Name.c_str(), nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    
    glfwMakeContextCurrent(m_Window);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetKeyCallback(m_Window, key_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);
    glfwSwapInterval(1); 

    
    InputManager::Init(m_Window);
    
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    m_Scene = std::make_unique<Scene>();
    m_Camera = std::make_unique<Camera>(m_Specification.Width, m_Specification.Height, glm::vec3(0.0f, 0.0f, 2.0f));
    
    
    glfwSetWindowUserPointer(m_Window, this);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    ResourceManager::LoadShader("default", "../shaders/core/default.vert", "../shaders/core/default.frag");
    ResourceManager::LoadShader("light", "../shaders/core/light.vert", "../shaders/core/light.frag");
    ResourceManager::LoadShader("skybox", "../shaders/environment/skybox.vert", "../shaders/environment/skybox.frag");
    ResourceManager::LoadShader("gradientSky", "../shaders/environment/gradient_sky.vert", "../shaders/environment/gradient_sky.frag");
    ResourceManager::LoadShader("water", "../shaders/environment/water.vert", "../shaders/environment/water.frag");
    ResourceManager::LoadShader("cloud2d", "../shaders/environment/2dcloud.vert", "../shaders/environment/2dcloud.frag");
    ResourceManager::LoadShader("volumetric_cloud", "../shaders/environment/volumetric_cloud.vert", "../shaders/environment/volumetric_cloud.frag");
    ResourceManager::LoadShader("hitbox", "../shaders/editor/hitbox.vert", "../shaders/editor/hitbox.frag");
    
    HitboxGraphics::Init();
    
    m_Water = std::make_unique<Water>();
    m_Cloud2D = std::make_unique<Cloud2D>();
    m_VolumetricCloud = std::make_unique<VolumetricCloud>();
    
    auto standardPipeline = std::make_unique<StandardPipeline>();
    standardPipeline->Init();
    m_RenderPipeline = std::move(standardPipeline);

    
    Texture defaultDiffuse = ResourceManager::LoadTexture("defaultDiffuse", "../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    Texture defaultSpecular = ResourceManager::LoadTexture("defaultSpecular", "../Resource/default/texture/DefaultTex.png", "specular", 1);

    

    
    
    
    
    Logger::AddLog("Application Initialized");

    m_Initialized = true;
    return true;
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

void Application::Run()
{
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(m_Window) && m_Running)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        m_RenderContext.time = currentFrame;
        m_RenderContext.deltaTime = deltaTime;

        glfwPollEvents();
        InputManager::Update();

        OnUpdate(deltaTime);
        
        m_Scene->Update(deltaTime);

        OnRender();

        glfwSwapBuffers(m_Window);
    }
}

void Application::OpenProject(const std::string& path) {
    m_ProjectRoot = path;
    Logger::AddLog("Opened Project: %s", path.c_str());
}

void Application::CreateProject(const std::string& path) {
    namespace fs = std::filesystem;
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            fs::create_directories(path + "/Assets");
            fs::create_directories(path + "/Scenes");
            fs::create_directories(path + "/Shaders");
        }
        OpenProject(path);
        Logger::AddLog("Created Project: %s", path.c_str());
    } catch (const std::exception& e) {
        Logger::AddLog("Error creating project: %s", e.what());
    }
}

void Application::Close()
{
    m_Running = false;
}

void Application::Shutdown()
{
    if (!m_Initialized) return;
    
    ResourceManager::Clear();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    
    m_Initialized = false;
}
