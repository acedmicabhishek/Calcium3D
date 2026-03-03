#include "EditorApplication.h"
#include "Editor.h"
#include "Logger.h"
#include "ResourceManager.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Physics/HitboxGraphics.h"
#include "../UI/UICreationEngine.h"
#include "../UI/Screens/GameplayScreen.h"

struct WindowData {
    Camera* camera;
    EditorLayer* editor;
    Application* app;
};

EditorApplication::EditorApplication(const ApplicationSpecification& spec)
    : Application(spec)
{
}

EditorApplication::~EditorApplication()
{
}

bool EditorApplication::Init()
{
    if (!Application::Init()) return false;
    
    m_EditorLayer = std::make_unique<EditorLayer>();
    m_EditorLayer->Init(m_Window);

    m_Home = std::make_unique<Home>();
    m_Home->Init(m_Window);
    m_Gizmo = std::make_unique<Gizmo>(); 

    static WindowData windowData;
    windowData.camera = m_Camera.get();
    windowData.editor = m_EditorLayer.get();
    windowData.app = this;
    glfwSetWindowUserPointer(m_Window, &windowData);
    
    ResourceManager::LoadShader("gizmo", "../shaders/editor/gizmo.vert", "../shaders/editor/gizmo.frag");

    CreateViewportFramebuffer(m_Specification.Width, m_Specification.Height);
    
    
    if (std::filesystem::exists("project.json")) {
        Logger::AddLog("Standalone project config found! Booting into Game Mode...");
        try {
            std::ifstream file("project.json");
            nlohmann::json config = nlohmann::json::parse(file);
            
            m_ProjectRoot = std::filesystem::current_path().string();
            m_EditorLayer->SetContentPath(m_ProjectRoot);
            
            if (config.contains("start_scene")) {
                std::string scenePath = m_ProjectRoot + "/Scenes/" + config["start_scene"].get<std::string>();
                m_Scene->Load(scenePath);
            }
            
            m_State = AppState::Editor;
        } catch (const std::exception& e) {
            Logger::AddLog("[ERROR] Failed to load standalone config: %s", e.what());
        }
    }

    return true;
}

void EditorApplication::SaveProject() {
    if (m_ProjectRoot.empty()) return;
    
    nlohmann::json config;
    std::filesystem::path currentScenePath = m_Scene->GetFilepath();
    
    if (!currentScenePath.empty()) {
        
        
        std::filesystem::path projRoot(m_ProjectRoot);
        std::string relPath = std::filesystem::relative(currentScenePath, projRoot).string();
        config["last_scene"] = relPath;
    } else {
        config["last_scene"] = "";
    }
    
    config["start_state"] = (int)Application::Get().m_StartGameState;
    
    std::string configPath = m_ProjectRoot + "/project.c3dproj";
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << config.dump(4);
        Logger::AddLog("Saved Project Configuration: %s", configPath.c_str());
    } else {
        Logger::AddLog("[ERROR] Failed to save Project Configuration: %s", configPath.c_str());
    }
}

void EditorApplication::OpenProject(const std::string& path) {
    m_ProjectRoot = path;
    m_State = AppState::Editor;
    m_EditorLayer->SetContentPath(path);
    
    m_Scene->Clear();
    UICreationEngine::Clear();
    m_EditorLayer->selectedCube = -1;
    m_EditorLayer->selectedMesh = -1;
    m_EditorLayer->isLightSelected = false;
    m_EditorLayer->selectedPointLightIndex = -1;

    Logger::AddLog("Opened Project: %s", path.c_str());

    std::string uiLayoutPath = path + "/ui_layout.json";
    if (std::filesystem::exists(uiLayoutPath)) {
        UICreationEngine::LoadLayout(uiLayoutPath);
        Logger::AddLog("Restored UI layout from %s", uiLayoutPath.c_str());
    }
    
    std::string configPath = path + "/project.c3dproj";
    if (std::filesystem::exists(configPath)) {
        try {
            std::ifstream file(configPath);
            nlohmann::json config = nlohmann::json::parse(file);
            
            if (config.contains("last_scene")) {
                std::string lastScene = config["last_scene"].get<std::string>();
                if (!lastScene.empty()) {
                    std::filesystem::path fullScenePath = std::filesystem::path(path) / lastScene;
                    if (std::filesystem::exists(fullScenePath)) {
                        m_Scene->Load(fullScenePath.string());
                        Logger::AddLog("Restored last active scene: %s", lastScene.c_str());
                    }
                }
            }
            
            if (config.contains("start_state")) {
                Application::Get().m_StartGameState = (GameState)config["start_state"].get<int>();
            }
        } catch (const std::exception& e) {
            Logger::AddLog("[ERROR] Failed to parse project.c3dproj: %s", e.what());
        }
    }
}

void EditorApplication::CreateProject(const std::string& path) {
    namespace fs = std::filesystem;
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
            fs::create_directories(path + "/Assets");
            fs::create_directories(path + "/Scenes");
            fs::create_directories(path + "/Shaders");
            fs::create_directories(path + "/Scripts");
            
            nlohmann::json config;
            config["last_scene"] = "";
            config["start_state"] = 0;
            std::ofstream file(path + "/project.c3dproj");
            if (file.is_open()) {
                file << config.dump(4);
            }
        }
        OpenProject(path);
        Logger::AddLog("Created Project: %s", path.c_str());
    } catch (const std::exception& e) {
        Logger::AddLog("Error creating project: %s", e.what());
    }
}

void EditorApplication::EnterPlayMode() {
    if (m_PlayMode) return;
    m_PlayMode = true;
    Editor::isEditMode = false;
    
    
    m_PlayModeSceneBackup = "/tmp/calcium3d_playmode_backup.scene";
    m_Scene->Save(m_PlayModeSceneBackup);
    
    for (auto& obj : m_Scene->GetObjects()) {
        if (obj.hasAudio && obj.audio.playOnAwake && !obj.audio.filePath.empty()) {
            obj.audio.playing = true;
        }
        for (auto& script : obj.behaviors) {
            if (script) {
                script->gameObject = &obj;
                script->OnStart();
            }
        }
    }

    
    Logger::AddLog("▶ Entered Play Mode");
}

void EditorApplication::ExitPlayMode() {
    if (!m_PlayMode) return;
    m_PlayMode = false;
    Editor::isEditMode = true;
    
    
    if (!m_PlayModeSceneBackup.empty() && std::filesystem::exists(m_PlayModeSceneBackup)) {
        std::string originalPath = m_Scene->GetFilepath(); 
        m_Scene->Clear();
        m_Scene->Load(m_PlayModeSceneBackup);
        m_Scene->SetFilepath(originalPath); 
        std::filesystem::remove(m_PlayModeSceneBackup);
        m_PlayModeSceneBackup.clear();
    }
    
    
    m_EditorLayer->selectedCube = -1;
    m_EditorLayer->selectedMesh = -1;
    m_EditorLayer->isLightSelected = false;
    m_EditorLayer->selectedPointLightIndex = -1;
    m_EditorLayer->m_PlayModeActiveScreen.clear();
    
    Logger::AddLog("■ Exited Play Mode — Scene Restored");
}

void EditorApplication::TogglePlayMode() {
    if (m_PlayMode) ExitPlayMode();
    else EnterPlayMode();
}

void EditorApplication::OnUpdate(float deltaTime)
{
    
    static bool f7Pressed = false;
    if (glfwGetKey(m_Window, GLFW_KEY_F7) == GLFW_PRESS) {
        if (!f7Pressed) {
            f7Pressed = true;
            TogglePlayMode();
        }
    } else {
        f7Pressed = false;
    }

    if (!m_EditorLayer->isTimePaused) {
        m_EditorLayer->timeOfDay += deltaTime * m_EditorLayer->timeSpeed;
        if (m_EditorLayer->timeOfDay >= 24.0f) m_EditorLayer->timeOfDay -= 24.0f;
        if (m_EditorLayer->timeOfDay < 0.0f) m_EditorLayer->timeOfDay += 24.0f;
    }

    if (Editor::isEditMode) {
         m_Camera->Inputs(m_Window, deltaTime);
    } else {
        
        
        m_Scene->Update(deltaTime);
    }

    m_ShowSkybox = m_EditorLayer->showSkybox;
    m_ShowGradientSky = m_EditorLayer->showGradientSky;
    m_ShowWater = m_EditorLayer->showWater;
    m_ShowClouds = m_EditorLayer->showClouds;

    if (m_MSAASamples != m_EditorLayer->msaaSamples) {
        CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
    }
}

void EditorApplication::OnRender()
{
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

    if (m_State == AppState::Home) {
        RenderHome();
    } else {
        RenderEditor(0.016f); 
    }
}

void EditorApplication::RenderHome() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    int winW, winH;
    glfwGetFramebufferSize(m_Window, &winW, &winH);
    glViewport(0, 0, winW, winH);
    glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_Home->Render();
}

void EditorApplication::RenderEditor(float deltaTime) {
    if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
    }
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_RenderContext.width = m_ViewportWidth;
    m_RenderContext.height = m_ViewportHeight;
    m_RenderContext.camera = m_Camera.get();
    m_RenderContext.scene = m_Scene.get();
    m_RenderContext.water = m_Water.get();
    m_RenderContext.cloud2d = m_Cloud2D.get();
    m_RenderContext.volCloud = m_VolumetricCloud.get();
    
    if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
        m_RenderContext.mainFBO = m_MSAAFBO;
    } else {
        m_RenderContext.mainFBO = m_ViewportFBO;
    }
    
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

    ProcessSceneCameras();

    if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
    }
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

    m_RenderContext.renderEditorObjects = true;
    m_RenderPipeline->Execute(m_RenderContext);
    HitboxGraphics::Render(*m_Scene, *m_Camera);

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
}

void EditorApplication::PostRender() {
    m_EditorLayer->End();
}

void EditorApplication::Shutdown() {
    m_EditorLayer->Shutdown();
    Application::Shutdown();
}

void EditorApplication::CreateMSAAFramebuffer(int samples) {
    if (m_MSAAFBO != 0) {
        glDeleteFramebuffers(1, &m_MSAAFBO);
        glDeleteTextures(1, &m_MSAAColorBuffer);
        glDeleteRenderbuffers(1, &m_MSAARBO);
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

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void EditorApplication::CreateViewportFramebuffer(int width, int height) {
    
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

