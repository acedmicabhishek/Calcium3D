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
#include <imgui.h>
#include "../Physics/HitboxGraphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb/stb_image.h"
#include "Pipelines/StandardPipeline.h"
#include "../UI/Screens/StartScreen.h"
#include "../UI/Screens/GameScreen.h"
#include "../UI/Screens/FallbackScreen.h"
#include "../UI/UIManager.h"
#include "../UI/UICreationEngine.h"
#include "../AudioEngine/AudioEngine.h"
#include "ThreadManager.h"

Application* Application::s_Instance = nullptr;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (Application::GetInstance() && Application::GetInstance()->GetWindow() == window) {
         
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
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

void Application::ChangeState(int newState) {
    GameStateManager::ChangeState(newState);
}



bool Application::Init()
{
    ThreadManager::Init();
    if (m_Initialized) return true;
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

    ResourceManager::LoadShader("default", "../shaders/passes/geometry/default.vert", "../shaders/passes/geometry/default.frag");
    ResourceManager::LoadShader("light", "../shaders/passes/geometry/light.vert", "../shaders/passes/geometry/light.frag");
    ResourceManager::LoadShader("skybox", "../shaders/passes/sky/skybox.vert", "../shaders/passes/sky/skybox.frag");
    ResourceManager::LoadShader("gradientSky", "../shaders/passes/sky/gradient_sky.vert", "../shaders/passes/sky/gradient_sky.frag");
    ResourceManager::LoadShader("water", "../shaders/passes/transparency/water.vert", "../shaders/passes/transparency/water.frag");
    ResourceManager::LoadShader("cloud2d", "../shaders/passes/transparency/2dcloud.vert", "../shaders/passes/transparency/2dcloud.frag");
    ResourceManager::LoadShader("volumetric_cloud", "../shaders/passes/transparency/volumetric_cloud.vert", "../shaders/passes/transparency/volumetric_cloud.frag");
    ResourceManager::LoadShader("hitbox", "../shaders/editor/hitbox.vert", "../shaders/editor/hitbox.frag");
    
    HitboxGraphics::Init();
    
    m_Water = std::make_unique<Water>();
    m_Cloud2D = std::make_unique<Cloud2D>();
    m_VolumetricCloud = std::make_unique<VolumetricCloud>();
    
    auto standardPipeline = std::make_unique<StandardPipeline>();
    standardPipeline->Init();
    m_RenderPipeline = std::move(standardPipeline);

    GameStateManager::Init();

    UICreationEngine::LoadLayout("ui_layout.json");
    
    GameStateManager::RegisterState<FallbackScreen>("Start Screen");
    GameStateManager::RegisterState<FallbackScreen>("Gameplay");
    GameStateManager::RegisterState<FallbackScreen>("Exit");
    GameStateManager::RegisterState<FallbackScreen>("Pause");
    GameStateManager::RegisterState<FallbackScreen>("Settings");
    
    GameStateManager::ChangeState((int)m_StartGameState);

    Texture defaultDiffuse = ResourceManager::LoadTexture("defaultDiffuse", "../Resource/default/texture/DefaultTex.png", "diffuse", 0);
    Texture defaultSpecular = ResourceManager::LoadTexture("defaultSpecular", "../Resource/default/texture/DefaultTex.png", "specular", 1);

    

    
    
    
    
    AudioEngine::Init();
    
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

        
        
        

        m_RenderContext.deltaTime = deltaTime;

        glfwPollEvents();
        InputManager::Update();

        GameStateManager::Update(deltaTime);
        
        OnUpdate(deltaTime);
        
        if (GameStateManager::IsState(GameState::GAMEPLAY)) {
            m_Scene->Update(deltaTime);
        }

        if (m_Camera && m_Scene) {
            AudioEngine::Update(m_Scene.get(), m_Camera->Position, m_Camera->Orientation, m_Camera->Up, deltaTime);
        }

        OnRender();

        
        #ifdef C3D_RUNTIME
        int w, h;
        glfwGetFramebufferSize(m_Window, &w, &h);
        ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
        GameStateManager::Render(glm::vec2(w, h), glm::vec2(mainPos.x, mainPos.y));
        #endif

        PostRender();

        glfwSwapBuffers(m_Window);
    }
}

void Application::OpenProject(const std::string& path) {
    m_ProjectRoot = path;
    Logger::AddLog("Opened Project: %s", path.c_str());
    
    std::string title = m_Specification.Name + " - " + GetProjectName();
    glfwSetWindowTitle(m_Window, title.c_str());
}

std::string Application::GetProjectName() const {
    if (m_ProjectRoot.empty()) return "Untitled Project";
    
    std::filesystem::path p(m_ProjectRoot);
    return p.filename().string();
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
    AudioEngine::Shutdown();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    ThreadManager::Shutdown();
    
    m_Initialized = false;
}

void Application::ProcessSceneCameras() {
    if (!m_Scene) return;
    
    auto& objects = m_Scene->GetObjects();
    for (auto& obj : objects) {
        if (obj.hasCamera && obj.camera.enabled) {
            
            if (obj.camera.fbo == 0) {
                glGenFramebuffers(1, &obj.camera.fbo);
                glBindFramebuffer(GL_FRAMEBUFFER, obj.camera.fbo);

                glGenTextures(1, &obj.camera.renderTexture);
                glBindTexture(GL_TEXTURE_2D, obj.camera.renderTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, obj.camera.resolutionX, obj.camera.resolutionY, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, obj.camera.renderTexture, 0);

                glGenRenderbuffers(1, &obj.camera.depthBuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, obj.camera.depthBuffer);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, obj.camera.resolutionX, obj.camera.resolutionY);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, obj.camera.depthBuffer);
            }

            
            Camera sceneCam(obj.camera.resolutionX, obj.camera.resolutionY, obj.position);
            sceneCam.Orientation = obj.rotation * glm::vec3(0,0,-1);
            sceneCam.Up = obj.rotation * glm::vec3(0,1,0);
            sceneCam.FOV = obj.camera.fov;
            sceneCam.nearPlane = obj.camera.nearPlane;
            sceneCam.farPlane = obj.camera.farPlane;

            
            glBindFramebuffer(GL_FRAMEBUFFER, obj.camera.fbo);
            glViewport(0, 0, obj.camera.resolutionX, obj.camera.resolutionY);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            RenderContext camCtx = m_RenderContext; 
            camCtx.mainFBO = obj.camera.fbo;
            camCtx.width = obj.camera.resolutionX;
            camCtx.height = obj.camera.resolutionY;
            camCtx.camera = &sceneCam;
            camCtx.renderEditorObjects = false;
            
            m_RenderPipeline->Execute(camCtx);
        }
    }
}
