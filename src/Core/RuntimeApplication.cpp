#include "RuntimeApplication.h"
#include "Logger.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include <fstream>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>

RuntimeApplication::RuntimeApplication(const ApplicationSpecification& spec)
    : Application(spec)
{
}

RuntimeApplication::~RuntimeApplication()
{
}

bool RuntimeApplication::Init()
{
    if (!Application::Init()) return false;
    
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    LoadProjectConfig();
    return true;
}

void RuntimeApplication::LoadProjectConfig() {
    if (std::filesystem::exists("project.json")) {
        Logger::AddLog("Standalone project config found! Booting into Game Mode...");
        try {
            std::ifstream file("project.json");
            nlohmann::json config = nlohmann::json::parse(file);
            
            m_ProjectRoot = std::filesystem::current_path().string();
            
            if (config.contains("start_scene")) {
                std::string scenePath = m_ProjectRoot + "/Scenes/" + config["start_scene"].get<std::string>();
                m_Scene->Load(scenePath);
            }
            
        } catch (const std::exception& e) {
            Logger::AddLog("[ERROR] Failed to load standalone config: %s", e.what());
        }
    } else {
        Logger::AddLog("[WARNING] project.json not found! Running with empty scene.");
    }
}

void RuntimeApplication::OnUpdate(float deltaTime)
{
    m_LastDeltaTime = deltaTime;
}

void RuntimeApplication::OnRender()
{
    
    if (GameStateManager::IsState(GameState::START_SCREEN)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        return;
    }

    int winW, winH;
    glfwGetFramebufferSize(m_Window, &winW, &winH);
    glViewport(0, 0, winW, winH);
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_RenderContext.width = winW;
    m_RenderContext.height = winH;
    m_RenderContext.camera = m_Camera.get();
    m_RenderContext.scene = m_Scene.get();
    m_RenderContext.water = m_Water.get();
    m_RenderContext.cloud2d = m_Cloud2D.get();
    m_RenderContext.volCloud = m_VolumetricCloud.get();
    
    m_RenderContext.mainFBO = 0; 
    
    m_RenderContext.time = glfwGetTime();
    m_RenderContext.deltaTime = m_LastDeltaTime;
    m_RenderContext.timeOfDay = 12.0f;
    m_RenderContext.showGradientSky = true;
    m_RenderContext.showSkybox = false;
    m_RenderContext.showWater = true;
    m_RenderContext.showClouds = true;
    m_RenderContext.cloudMode = 1;
    m_RenderContext.waterHeight = 0.0f;
    m_RenderContext.cloudHeight = 50.0f;
    m_RenderContext.cloudDensity = 0.5f;
    m_RenderContext.cloudCover = 0.5f;
    m_RenderContext.waveSpeed = 1.0f;
    m_RenderContext.waveStrength = 0.1f;
    m_RenderContext.waterColor = glm::vec3(0.1f, 0.4f, 0.6f);
    m_RenderContext.sunEnabled = true;
    m_RenderContext.sunPosition = glm::vec3(0.0f, 10.0f, 0.0f);
    m_RenderContext.sunColor = glm::vec4(1.0f);
    m_RenderContext.sunIntensity = 1.0f;
    m_RenderContext.moonEnabled = false;

    m_RenderPipeline->Execute(m_RenderContext);

    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RuntimeApplication::PostRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
