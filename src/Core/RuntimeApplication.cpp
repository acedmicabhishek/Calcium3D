#include "RuntimeApplication.h"
#include "Logger.h"
#include "Water.h"
#include "2dCloud.h"
#include "VolumetricCloud.h"
#include "ObjectFactory.h"
#include "../UI/UICreationEngine.h"
#include <fstream>
#include <iostream>
#include <map>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include "../Physics/PhysicsEngine.h"
#include "../Physics/HitboxGraphics.h"
#include "../Core/InputManager.h"
#include "../UI/Screens/StartScreen.h"
#include "../UI/Screens/GameplayScreen.h"

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

    m_Console = std::make_unique<Console>();
    m_Console->Init();

    LoadProjectConfig();
    return true;
}

void RuntimeApplication::LoadProjectConfig() {
    bool sceneLoaded = false;

    if (std::filesystem::exists("project.json")) {
        Logger::AddLog("Standalone project config found! Booting into Game Mode...");
        try {
            std::ifstream file("project.json");
            nlohmann::json config = nlohmann::json::parse(file);
            
            m_ProjectRoot = std::filesystem::current_path().string();
            
            
            std::string uiLayoutPath = m_ProjectRoot + "/ui_layout.json";
            if (std::filesystem::exists(uiLayoutPath)) {
                UICreationEngine::LoadLayout(uiLayoutPath);
                Logger::AddLog("Loaded UI Layout: %s", uiLayoutPath.c_str());
            } else {
                Logger::AddLog("[WARNING] No UI layout found at %s", uiLayoutPath.c_str());
            }
            
            if (config.contains("start_scene")) {
                std::string scenePath = m_ProjectRoot + "/Scenes/" + config["start_scene"].get<std::string>();
                if (std::filesystem::exists(scenePath)) {
                    m_Scene->Load(scenePath);
                    sceneLoaded = true;
                } else {
                    Logger::AddLog("[WARNING] Scene not found: %s", scenePath.c_str());
                }
            }
            
            if (config.contains("environment")) {
                auto& env = config["environment"];
                if (env.contains("showSkybox")) m_Console->SetSkyboxEnabled(env["showSkybox"]);
                if (env.contains("showGradientSky")) m_Console->SetGradientSkyEnabled(env["showGradientSky"]);
                if (env.contains("showWater")) m_Console->SetWaterEnabled(env["showWater"]);
                if (env.contains("showClouds")) m_Console->SetCloudsEnabled(env["showClouds"]);
                if (env.contains("cloudMode")) m_RenderContext.cloudMode = env["cloudMode"];
                if (env.contains("waterHeight")) m_RenderContext.waterHeight = env["waterHeight"];
                if (env.contains("cloudHeight")) m_RenderContext.cloudHeight = env["cloudHeight"];
                if (env.contains("cloudDensity")) m_RenderContext.cloudDensity = env["cloudDensity"];
                if (env.contains("cloudCover")) m_RenderContext.cloudCover = env["cloudCover"];
                if (env.contains("waveSpeed")) m_RenderContext.waveSpeed = env["waveSpeed"];
                if (env.contains("waveStrength")) m_RenderContext.waveStrength = env["waveStrength"];
                if (env.contains("waterColor")) {
                    auto& c = env["waterColor"];
                    m_RenderContext.waterColor = glm::vec3(c[0], c[1], c[2]);
                }
                
                if (env.contains("timeOfDay")) m_RenderContext.timeOfDay = env["timeOfDay"];
                if (env.contains("sunEnabled")) m_RenderContext.sunEnabled = env["sunEnabled"];
                if (env.contains("sunIntensity")) m_RenderContext.sunIntensity = env["sunIntensity"];
                if (env.contains("sunColor")) {
                    auto& sc = env["sunColor"];
                    m_RenderContext.sunColor = glm::vec4(sc[0], sc[1], sc[2], sc[3]);
                }
                if (env.contains("sunBloom")) m_RenderContext.sunBloom = env["sunBloom"];
                
                if (env.contains("moonEnabled")) m_RenderContext.moonEnabled = env["moonEnabled"];
                if (env.contains("moonIntensity")) m_RenderContext.moonIntensity = env["moonIntensity"];
                if (env.contains("moonColor")) {
                    auto& mc = env["moonColor"];
                    m_RenderContext.moonColor = glm::vec4(mc[0], mc[1], mc[2], mc[3]);
                }
                if (env.contains("moonBloom")) m_RenderContext.moonBloom = env["moonBloom"];
                
                if (env.contains("globalTilingFactor")) m_RenderContext.globalTilingFactor = env["globalTilingFactor"];
                
                if (env.contains("camera")) {
                    auto& cam = env["camera"];
                    if (cam.contains("position")) {
                        auto& p = cam["position"];
                        m_Camera->Position = glm::vec3(p[0], p[1], p[2]);
                    }
                    if (cam.contains("orientation")) {
                        auto& o = cam["orientation"];
                        m_Camera->Orientation = glm::vec3(o[0], o[1], o[2]);
                    }
                    if (cam.contains("yaw")) m_Camera->yaw = cam["yaw"];
                    if (cam.contains("pitch")) m_Camera->pitch = cam["pitch"];
                    if (cam.contains("fov")) m_Camera->FOV = cam["fov"];
                }
                
                Logger::AddLog("Loaded extracted Editor Environment Settings.");
            }

            if (config.contains("game_states")) {
                auto& jStates = config["game_states"];
                for (auto& item : jStates.items()) {
                    int id = std::stoi(item.key());
                    std::string name = item.value().get<std::string>();
                    GameStateManager::RegisterState(id, name);
                    
                    if (name.find("Start Screen") != std::string::npos || name.find("Start Menu") != std::string::npos) {
                        AddScreen(id, std::make_unique<StartScreen>());
                    } else if (name.find("Gameplay") != std::string::npos || name.find("Main") != std::string::npos || name.find("Custom") != std::string::npos || id == 1) {
                        AddScreen(id, std::make_unique<GameplayScreen>());
                    } else {
                        
                        AddScreen(id, std::make_unique<GameplayScreen>());
                    }
                }
            }

            if (config.contains("start_state")) {
                int startState = config["start_state"].get<int>();
                ChangeState(startState);
            } else {
                ChangeState((int)GameState::GAMEPLAY);
            }
            
            
            for (auto& pair : m_Screens) {
                if (pair.second) pair.second->Init();
            }
            
            
            m_ActiveScreen = m_Screens[GameStateManager::GetState()].get();
            
        } catch (const std::exception& e) {
            Logger::AddLog("[ERROR] Failed to load standalone config: %s", e.what());
            ChangeState((int)GameState::GAMEPLAY);
        }
    } else {
        Logger::AddLog("[WARNING] project.json not found!");
        ChangeState((int)GameState::GAMEPLAY);
    }

    
    if (!sceneLoaded || m_Scene->GetObjects().empty()) {
        Logger::AddLog("Loading default demo scene...");
        CreateDefaultScene();
    }

    
    for (auto& obj : m_Scene->GetObjects()) {
        for (auto& script : obj.behaviors) {
            if (script) {
                script->gameObject = &obj;
                script->OnStart();
            }
        }
    }
}

void RuntimeApplication::CreateDefaultScene() {
    
    Mesh planeMesh = ObjectFactory::createPlane();
    GameObject ground(std::move(planeMesh), "Ground");
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.scale = glm::vec3(20.0f, 1.0f, 20.0f);
    ground.isStatic = true;
    m_Scene->AddObject(std::move(ground));

    
    Mesh cubeMesh1 = ObjectFactory::createCube();
    GameObject cube1(std::move(cubeMesh1), "Cube");
    cube1.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube1.scale = glm::vec3(1.0f);
    m_Scene->AddObject(std::move(cube1));

    
    Mesh cubeMesh2 = ObjectFactory::createCube();
    GameObject cube2(std::move(cubeMesh2), "Cube 2");
    cube2.position = glm::vec3(3.0f, 0.0f, -2.0f);
    cube2.scale = glm::vec3(0.7f);
    m_Scene->AddObject(std::move(cube2));

    
    Mesh sphereMesh = ObjectFactory::createSphere(32, 16);
    GameObject sphere(std::move(sphereMesh), "Sphere");
    sphere.position = glm::vec3(-2.5f, 0.5f, -1.0f);
    sphere.scale = glm::vec3(1.0f);
    sphere.shape = ColliderShape::Sphere;
    m_Scene->AddObject(std::move(sphere));

    
    Mesh cubeMesh3 = ObjectFactory::createCube();
    GameObject pillar(std::move(cubeMesh3), "Pillar");
    pillar.position = glm::vec3(-5.0f, 1.0f, 3.0f);
    pillar.scale = glm::vec3(0.5f, 3.0f, 0.5f);
    pillar.isStatic = true;
    m_Scene->AddObject(std::move(pillar));

    
    Scene::PointLight* light = m_Scene->CreatePointLight();
    if (light) {
        light->position = glm::vec3(2.0f, 4.0f, 1.0f);
        light->color = glm::vec4(1.0f, 0.9f, 0.7f, 1.0f);
        light->intensity = 8.0f;
        light->enabled = true;
        light->constant = 1.0f;
        light->linear = 0.09f;
        light->quadratic = 0.032f;
    }

    
    Scene::PointLight* light2 = m_Scene->CreatePointLight();
    if (light2) {
        light2->position = glm::vec3(-3.0f, 3.0f, -2.0f);
        light2->color = glm::vec4(0.4f, 0.6f, 1.0f, 1.0f);
        light2->intensity = 5.0f;
        light2->enabled = true;
        light2->constant = 1.0f;
        light2->linear = 0.09f;
        light2->quadratic = 0.032f;
    }

    Logger::AddLog("Default scene created: ground, 3 cubes, 1 sphere, 2 lights");
}

void RuntimeApplication::OnUpdate(float deltaTime)
{
    m_LastDeltaTime = deltaTime;
    
    
    if (m_Scene) {
        m_Scene->Update(deltaTime);
    }

    
    if (InputManager::IsKeyJustPressed(GLFW_KEY_GRAVE_ACCENT)) {
        m_Console->Toggle();
    }

    
    if (m_Console && !m_Console->IsTimePaused()) {
        float speed = m_Console->GetTimeSpeed();
        m_RenderContext.timeOfDay += deltaTime * speed;
        if (m_RenderContext.timeOfDay >= 24.0f) m_RenderContext.timeOfDay -= 24.0f;
        if (m_RenderContext.timeOfDay < 0.0f)  m_RenderContext.timeOfDay += 24.0f;
    }
}

void RuntimeApplication::OnRender()
{
    int winW, winH;
    glfwGetFramebufferSize(m_Window, &winW, &winH);
    glViewport(0, 0, winW, winH);

    if (m_Camera) {
        m_Camera->width = winW;
        m_Camera->height = winH;
    }

    
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
    
    
    m_RenderContext.showSkybox = m_Console->IsSkyboxEnabled();
    m_RenderContext.showGradientSky = m_Console->IsGradientSkyEnabled();
    m_RenderContext.showWater = m_Console->IsWaterEnabled();
    m_RenderContext.showClouds = m_Console->IsCloudsEnabled();
    
    float angle = (m_RenderContext.timeOfDay - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
    glm::vec3 dynamicSunPos = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
    m_RenderContext.sunPosition = dynamicSunPos;
    
    glm::vec3 dynamicMoonPos = glm::vec3(-dynamicSunPos.x, -dynamicSunPos.y, 0.0f);
    m_RenderContext.moonPosition = dynamicMoonPos;
    
    m_RenderContext.msaaSamples = 4;
    m_RenderContext.msaaSkyPass = true;
    m_RenderContext.msaaGeometryPass = true;
    
    m_RenderPipeline->Execute(m_RenderContext);

    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RuntimeApplication::PostRender() {
    m_Console->Render(m_Camera.get(), m_RenderContext);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
