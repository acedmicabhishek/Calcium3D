#include "RuntimeApplication.h"
#include "Logger.h"
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
#include "../Scene/SceneManager.h"

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
    Logger::SetRuntimeConsole(m_Console.get());
    m_Console->Init();

    
    int winW, winH;
    glfwGetFramebufferSize(m_Window, &winW, &winH);
    CreateViewportFramebuffer(winW, winH);

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
            
            if (config.contains("disable_state_warning")) m_ShowStateWarning = !config["disable_state_warning"];
            
            if (config.contains("ms_control")) m_Console->SetMasterControl(config["ms_control"]);
            if (config.contains("hitboxes")) m_Console->SetHitboxEnabled(config["hitboxes"]);
            
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
                if (env.contains("showClouds")) m_Console->SetCloudsEnabled(env["showClouds"]);
                
                if (env.contains("cloudMode")) m_RenderContext.cloudMode = env["cloudMode"];
                if (env.contains("cloudHeight")) m_RenderContext.cloudHeight = env["cloudHeight"];
                if (env.contains("cloudDensity")) m_RenderContext.cloudDensity = env["cloudDensity"];
                if (env.contains("cloudCover")) m_RenderContext.cloudCover = env["cloudCover"];
                
                if (env.contains("cloudSpeed") && m_Cloud2D) m_Cloud2D->cloudSpeed = env["cloudSpeed"];
                if (env.contains("cloudTiling") && m_Cloud2D) m_Cloud2D->tiling = env["cloudTiling"];
                if (env.contains("cloudSize") && m_Cloud2D) m_Cloud2D->cloudSize = env["cloudSize"];
                if (env.contains("cloudRandomness") && m_Cloud2D) m_Cloud2D->randomness = env["cloudRandomness"];
                if (env.contains("cloudColor") && m_Cloud2D) {
                    auto& clc = env["cloudColor"];
                    m_Cloud2D->cloudColor = glm::vec3(clc[0], clc[1], clc[2]);
                }
                
                if (env.contains("timeOfDay")) m_RenderContext.timeOfDay = env["timeOfDay"];
                if (env.contains("sunEnabled")) m_RenderContext.sunEnabled = env["sunEnabled"];
                if (env.contains("sunIntensity")) m_RenderContext.sunIntensity = env["sunIntensity"];
                if (env.contains("sunColor")) {
                    auto& sc = env["sunColor"];
                    m_RenderContext.sunColor = glm::vec4(sc[0], sc[1], sc[2], sc[3]);
                }
                if (env.contains("sunBloom")) m_RenderContext.sunBloom = env["sunBloom"];
                
                if (env.contains("enableShadows")) m_RenderContext.enableShadows = env["enableShadows"];
                if (env.contains("enablePointShadows")) m_RenderContext.enablePointShadows = env["enablePointShadows"];
                if (env.contains("shadowBias")) m_RenderContext.shadowBias = env["shadowBias"];
                
                if (env.contains("moonEnabled")) m_RenderContext.moonEnabled = env["moonEnabled"];
                if (env.contains("moonIntensity")) m_RenderContext.moonIntensity = env["moonIntensity"];
                if (env.contains("moonColor")) {
                    auto& mc = env["moonColor"];
                    m_RenderContext.moonColor = glm::vec4(mc[0], mc[1], mc[2], mc[3]);
                }
                if (env.contains("moonBloom")) m_RenderContext.moonBloom = env["moonBloom"];
                
                if (env.contains("globalTilingFactor")) m_RenderContext.globalTilingFactor = env["globalTilingFactor"];
            }

            if (config.contains("graphics")) {
                auto& gfx = config["graphics"];
                if (gfx.contains("reflectionMode")) m_RenderContext.reflectionMode = gfx["reflectionMode"];
                if (gfx.contains("ssrUseCubemapFallback")) m_RenderContext.ssrUseCubemapFallback = gfx["ssrUseCubemapFallback"];
                if (gfx.contains("ssrGeometry")) m_RenderContext.ssrGeometry = gfx["ssrGeometry"];
                if (gfx.contains("ssrTransparency")) m_RenderContext.ssrTransparency = gfx["ssrTransparency"];
                if (gfx.contains("ssrAll")) m_RenderContext.ssrAll = gfx["ssrAll"];
                if (gfx.contains("ssrResolution")) m_RenderContext.ssrResolution = gfx["ssrResolution"];
                if (gfx.contains("ssrMaxSteps")) m_RenderContext.ssrMaxSteps = gfx["ssrMaxSteps"];
                if (gfx.contains("ssrMaxDistance")) m_RenderContext.ssrMaxDistance = gfx["ssrMaxDistance"];
                if (gfx.contains("ssrThickness")) m_RenderContext.ssrThickness = gfx["ssrThickness"];
                if (gfx.contains("ssrRenderDistance")) m_RenderContext.ssrRenderDistance = gfx["ssrRenderDistance"];
                if (gfx.contains("ssrFadeStart")) m_RenderContext.ssrFadeStart = gfx["ssrFadeStart"];
                if (gfx.contains("msaaSamples")) {
                    m_MSAASamples = gfx["msaaSamples"];
                    CreateMSAAFramebuffer(m_MSAASamples);
                }
            }
            
            if (config.contains("camera")) {
                auto& cam = config["camera"];
                if (cam.contains("position")) {
                    auto& p = cam["position"];
                    m_Camera->Position = glm::vec3(p[0], p[1], p[2]);
                }
                if (cam.contains("orientation")) {
                    auto& o = cam["orientation"];
                    m_Camera->Orientation = glm::vec3(o[0], o[1], o[2]);
                }
                if (cam.contains("up")) {
                    auto& u = cam["up"];
                    m_Camera->Up = glm::vec3(u[0], u[1], u[2]);
                }
                if (cam.contains("fov")) m_Camera->FOV = cam["fov"];
                if (cam.contains("nearPlane")) m_Camera->nearPlane = cam["nearPlane"];
                if (cam.contains("farPlane")) m_Camera->farPlane = cam["farPlane"];
            }
            
            Logger::AddLog("Loaded extracted Editor Environment & Graphics Settings.");

            if (config.contains("game_states")) {
                auto& jStates = config["game_states"];
                for (auto& item : jStates.items()) {
                    int id = std::stoi(item.key());
                    std::string name = item.value().get<std::string>();
                    GameStateManager::RegisterState(id, name);
                    
                    if (name.find("Start Screen") != std::string::npos || name.find("StartScreen") != std::string::npos || name.find("Start Menu") != std::string::npos) {
                        StateManager::RegisterState<StartScreen>(name);
                    } else if (name.find("Gameplay") != std::string::npos || name.find("Main") != std::string::npos || name.find("Custom") != std::string::npos || id == 1) {
                        StateManager::RegisterState<GameplayScreen>(name);
                    } else {
                        StateManager::RegisterState<GameplayScreen>(name);
                    }
                }
                
                
                auto registered = StateManager::GetRegisteredStateNames();
                Logger::AddLog("[State] Total registered state types: %d", (int)registered.size());
                for(const auto& r : registered) Logger::AddLog("  - Native Logic Registered: '%s'", r.c_str());
            }

            if (config.contains("start_state")) {
                m_StartGameState = (GameState)config["start_state"].get<int>();
            } else {
                m_StartGameState = GameState::START_SCREEN;
            }
            
            if (config.contains("disable_state_warning")) {
                m_DisableStateWarning = config["disable_state_warning"].get<bool>();
            }

        } catch (const std::exception& e) {
            Logger::AddLog("[ERROR] Failed to load standalone config: %s", e.what());
            m_StartGameState = GameState::START_SCREEN;
        }
    } else {
        Logger::AddLog("[WARNING] project.json not found!");
        m_StartGameState = GameState::START_SCREEN;
    }

    
    StateManager::ChangeState(m_StartGameState);

    
    if (!m_DisableStateWarning) {
        auto& allStates = GameStateManager::GetAllStates();
        if (allStates.empty()) {
            m_ShowStateWarning = true;
            Logger::AddLog("[WARNING] No game states are registered! Showing fallback screen.");
        } else {
            bool startStateFound = false;
            for (auto const& [id, name] : allStates) {
                if (id == (int)m_StartGameState) { startStateFound = true; break; }
            }
            if (!startStateFound) {
                m_ShowStateWarning = true;
                Logger::AddLog("[WARNING] Start state ID %d not found in registered states! Showing fallback screen.", (int)m_StartGameState);
            }
        }

        
        if (!sceneLoaded || m_Scene->GetObjects().empty()) {
            m_ShowStateWarning = true;
            Logger::AddLog("[WARNING] Scene failed to load — states may be misconfigured. Showing fallback screen.");
        }

        
        std::string uiCheck = m_ProjectRoot + "/ui_layout.json";
        if (!std::filesystem::exists(uiCheck)) {
            m_ShowStateWarning = true;
            Logger::AddLog("[WARNING] ui_layout.json not found — states may be misconfigured.");
        }
    }

    if (!sceneLoaded || m_Scene->GetObjects().empty()) {
        Logger::AddLog("Loading default demo scene...");
        CreateDefaultScene();
        m_IsDemoScene = true;
        m_CursorCaptured = false; 
        PhysicsEngine::GlobalPhysicsEnabled = false; 
    }

    
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

    
    SceneManager::Get().SetActiveScene(m_Scene.get());
    SceneManager::Get().SetMainCamera(m_Camera.get());
}

void RuntimeApplication::CreateDefaultScene() {
    
    Mesh planeMesh = ObjectFactory::createPlane();
    GameObject ground(std::move(planeMesh), "Ground");
    ground.position = glm::vec3(0.0f, -1.0f, 0.0f);
    ground.scale = glm::vec3(60.0f, 1.0f, 60.0f);
    ground.isStatic = true;
    m_Scene->AddObject(std::move(ground));

    
    for (int i = 0; i < 8; i++) {
        float angle = (float)i / 8.0f * 6.283185f;
        float r = 10.0f;
        Mesh cm = ObjectFactory::createCube();
        std::string name = "OrbitCube_" + std::to_string(i);
        GameObject obj(std::move(cm), name);
        obj.position = glm::vec3(cos(angle) * r, 1.0f, sin(angle) * r);
        obj.scale = glm::vec3(0.7f);
        obj.isStatic = true;
        m_Scene->AddObject(std::move(obj));
    }

    
    for (int i = 0; i < 3; i++) {
        Mesh sm = ObjectFactory::createSphere(32, 16);
        std::string name = "FloatSphere_" + std::to_string(i);
        GameObject obj(std::move(sm), name);
        float a = (float)i / 3.0f * 6.283185f;
        obj.position = glm::vec3(cos(a) * 4.0f, 3.0f + i * 3.0f, sin(a) * 4.0f);
        obj.scale = glm::vec3(0.6f + i * 0.2f);
        obj.isStatic = true;
        m_Scene->AddObject(std::move(obj));
    }

    
    {
        Mesh cm = ObjectFactory::createCube();
        GameObject obj(std::move(cm), "CenterCube");
        obj.position = glm::vec3(0.0f, 3.0f, 0.0f);
        obj.scale = glm::vec3(1.0f);
        obj.isStatic = true;
        m_Scene->AddObject(std::move(obj));
    }

    
    for (int i = 0; i < 4; i++) {
        Mesh cm = ObjectFactory::createCube();
        std::string name = "Pillar_" + std::to_string(i);
        GameObject obj(std::move(cm), name);
        float x = (i < 2) ? -15.0f : 15.0f;
        float z = (i % 2 == 0) ? -15.0f : 15.0f;
        obj.position = glm::vec3(x, 2.5f, z);
        obj.scale = glm::vec3(0.5f, 6.0f, 0.5f);
        obj.isStatic = true;
        m_Scene->AddObject(std::move(obj));
    }

    
    const glm::vec4 lightColors[] = {
        glm::vec4(1.0f, 0.3f, 0.2f, 1.0f),
        glm::vec4(0.2f, 1.0f, 0.4f, 1.0f),
        glm::vec4(0.3f, 0.5f, 1.0f, 1.0f)
    };
    for (int i = 0; i < 3; i++) {
        Scene::PointLight* light = m_Scene->CreatePointLight();
        if (light) {
            float a = (float)i / 3.0f * 6.283185f;
            light->position = glm::vec3(cos(a) * 8.0f, 4.0f, sin(a) * 8.0f);
            light->color = lightColors[i];
            light->intensity = 3.0f;
            light->enabled = true;
            light->constant = 1.0f;
            light->linear = 0.09f;
            light->quadratic = 0.032f;
        }
    }

    
    if (m_Camera) {
        m_Camera->Position = glm::vec3(0.0f, 6.0f, 20.0f);
        m_Camera->Orientation = glm::normalize(glm::vec3(0.0f, -0.25f, -1.0f));
        m_Camera->yaw = -90.0f;
        m_Camera->pitch = -14.0f;
    }

    Logger::AddLog("Demo scene created: ground, 8 orbit cubes, 3 spheres, center cube, 4 pillars, 3 lights");
}

void RuntimeApplication::OnUpdate(float deltaTime)
{
    m_LastDeltaTime = deltaTime;
    
    SceneManager::Get().Update(deltaTime); 
    
    if (m_Scene) {
        m_Scene->Update(deltaTime, (float)glfwGetTime());
    }

    
    if (m_IsDemoScene && m_Window) {
        if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !m_EscWasPressed) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_CursorCaptured = false;
            if (m_Camera) m_Camera->m_cameraEnabled = false;
        }
        m_EscWasPressed = (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

        if (!m_CursorCaptured && glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            m_CursorCaptured = true;
            if (m_Camera) {
                m_Camera->m_cameraEnabled = true;
                m_Camera->firstClick = true;
                glfwSetCursorPos(m_Window, m_Camera->width / 2, m_Camera->height / 2);
            }
        }
    }

    
    if ((m_IsDemoScene || m_Console->IsMasterControlEnabled()) && m_Camera && m_Window && m_CursorCaptured) {
        float camSpeed = 8.0f * deltaTime;
        if (m_Console->IsMasterControlEnabled()) camSpeed = 15.0f * deltaTime; 
        
        glm::vec3 forward = m_Camera->Orientation;
        glm::vec3 right = glm::normalize(glm::cross(forward, m_Camera->Up));
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
            m_Camera->Position += camSpeed * forward;
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
            m_Camera->Position -= camSpeed * forward;
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
            m_Camera->Position -= camSpeed * right;
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
            m_Camera->Position += camSpeed * right;
        if (glfwGetKey(m_Window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_Camera->Position += camSpeed * m_Camera->Up;
        if (glfwGetKey(m_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            m_Camera->Position -= camSpeed * m_Camera->Up;

        
        double mx, my;
        glfwGetCursorPos(m_Window, &mx, &my);
        float centerX = m_Camera->width / 2.0f;
        float centerY = m_Camera->height / 2.0f;
        float rotY = m_Camera->sensitivity * (float)(mx - centerX) / m_Camera->width;
        float rotX = m_Camera->sensitivity * (float)(my - centerY) / m_Camera->height;
        m_Camera->yaw += rotY;
        m_Camera->pitch -= rotX;
        if (m_Camera->pitch > 89.0f) m_Camera->pitch = 89.0f;
        if (m_Camera->pitch < -89.0f) m_Camera->pitch = -89.0f;
        glm::vec3 dir;
        dir.x = cos(glm::radians(m_Camera->yaw)) * cos(glm::radians(m_Camera->pitch));
        dir.y = sin(glm::radians(m_Camera->pitch));
        dir.z = sin(glm::radians(m_Camera->yaw)) * cos(glm::radians(m_Camera->pitch));
        m_Camera->Orientation = glm::normalize(dir);
        glfwSetCursorPos(m_Window, centerX, centerY);
    }

    
    if (InputManager::IsKeyJustPressed(GLFW_KEY_F1)) {
        m_ShowStateWarning = !m_ShowStateWarning;
    }

    
    if (m_IsDemoScene && m_Scene) {
        m_DemoTime += deltaTime;
        auto& objects = m_Scene->GetObjects();
        for (auto& obj : objects) {
            
            if (obj.name.rfind("OrbitCube_", 0) == 0) {
                int idx = std::stoi(obj.name.substr(10));
                float baseAngle = (float)idx / 8.0f * 6.283185f;
                float angle = baseAngle + m_DemoTime * 0.4f;
                float r = 10.0f + sin(m_DemoTime * 1.5f + idx) * 0.8f;
                obj.position.x = cos(angle) * r;
                obj.position.z = sin(angle) * r;
                obj.position.y = 1.0f + sin(m_DemoTime * 2.0f + idx * 0.7f) * 0.6f;
                
                float yAngle = glm::radians(m_DemoTime * 60.0f);
                obj.rotation = glm::angleAxis(yAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            
            else if (obj.name.rfind("FloatSphere_", 0) == 0) {
                int idx = std::stoi(obj.name.substr(12));
                float a = (float)idx / 3.0f * 6.283185f + m_DemoTime * 0.5f;
                obj.position.x = cos(a) * 4.0f;
                obj.position.z = sin(a) * 4.0f;
                obj.position.y = 3.0f + idx * 3.0f + sin(m_DemoTime * 1.2f + idx) * 1.2f;
                float s = 0.6f + idx * 0.2f + sin(m_DemoTime * 1.0f) * 0.08f;
                obj.scale = glm::vec3(s);
            }
            
            else if (obj.name == "CenterCube") {
                float rx = glm::radians(m_DemoTime * 35.0f);
                float ry = glm::radians(m_DemoTime * 50.0f);
                float rz = glm::radians(m_DemoTime * 25.0f);
                obj.rotation = glm::angleAxis(ry, glm::vec3(0,1,0))
                             * glm::angleAxis(rx, glm::vec3(1,0,0))
                             * glm::angleAxis(rz, glm::vec3(0,0,1));
                obj.position.y = 3.0f + sin(m_DemoTime * 1.0f) * 0.8f;
            }
        }
        
        auto& lights = m_Scene->GetPointLights();
        for (int i = 0; i < (int)lights.size() && i < 3; i++) {
            float a = (float)i / 3.0f * 6.283185f + m_DemoTime * 0.6f;
            lights[i].position.x = cos(a) * 8.0f;
            lights[i].position.z = sin(a) * 8.0f;
            lights[i].position.y = 4.0f + sin(m_DemoTime * 1.5f + i) * 2.0f;
        }
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
    
    
    if (winW != m_ViewportWidth || winH != m_ViewportHeight) {
        ResizeViewportFramebuffer(winW, winH);
        if (m_MSAASamples > 0) ResizeMSAAFramebuffer(winW, winH);
    }

    glViewport(0, 0, winW, winH);

    if (m_Camera) {
        m_Camera->width = winW;
        m_Camera->height = winH;
    }

    
    m_RenderContext.width = winW;
    m_RenderContext.height = winH;
    m_RenderContext.camera = m_Camera.get();
    m_RenderContext.scene = m_Scene.get();
    m_RenderContext.cloud2d = m_Cloud2D.get();
    m_RenderContext.volCloud = m_VolumetricCloud.get();
    m_RenderContext.time = glfwGetTime();
    m_RenderContext.deltaTime = m_LastDeltaTime;
    
    
    m_RenderContext.showSkybox = m_Console->IsSkyboxEnabled();
    m_RenderContext.showGradientSky = m_Console->IsGradientSkyEnabled();
    m_RenderContext.showClouds = m_Console->IsCloudsEnabled();

    
    unsigned int activeFBO = (m_MSAASamples > 0) ? m_MSAAFBO : m_ViewportFBO;
    m_RenderContext.mainFBO = activeFBO;

    
    glBindFramebuffer(GL_FRAMEBUFFER, activeFBO);
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    float angle = (m_RenderContext.timeOfDay - 6.0f) / 24.0f * 2.0f * glm::pi<float>();
    m_RenderContext.sunPosition = glm::vec3(cos(angle) * 10.0f, sin(angle) * 10.0f, 0.0f);
    m_RenderContext.moonPosition = glm::vec3(-m_RenderContext.sunPosition.x, -m_RenderContext.sunPosition.y, 0.0f);
    
    m_RenderContext.msaaSamples = m_MSAASamples;
    m_RenderContext.msaaSkyPass = true;
    m_RenderContext.msaaGeometryPass = true;
    
    ProcessSceneCameras();
    
    m_RenderPipeline->Execute(m_RenderContext);

    
    if (m_MSAASamples > 0) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_MSAAFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ViewportFBO);
        glBlitFramebuffer(0, 0, winW, winH, 0, 0, winW, winH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_ViewportFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, winW, winH, 0, 0, winW, winH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (m_Console && m_Console->IsHitboxEnabled()) {
        Renderer::RenderHitboxes(*m_Scene, *m_Camera);
    }
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RuntimeApplication::PostRender() {
    
    if (m_Scene) {
        for (auto& obj : m_Scene->GetObjects()) {
            if (!obj.isActive) continue;
            for (auto& behavior : obj.behaviors) {
                if (behavior && behavior->enabled) {
                    behavior->OnUI();
                }
            }
        }
    }

    if (m_ShowStateWarning) {
        ImGuiIO& io = ImGui::GetIO();
        float winW = 460.0f, winH = 300.0f;
        ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - winW) * 0.5f,
                                       (io.DisplaySize.y - winH) * 0.5f),
                                ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(winW, winH));
        ImGui::SetNextWindowBgAlpha(0.88f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.05f, 0.05f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.3f, 0.2f, 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::Begin("##StateWarning", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse);

        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.2f, 1.0f));
        ImGui::SetWindowFontScale(2.0f);
        float titleW = ImGui::CalcTextSize("!! FALLBACK SCREEN !!").x;
        ImGui::SetCursorPosX((winW - titleW) * 0.5f);
        ImGui::Text("!! FALLBACK SCREEN !!");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.4f, 1.0f));
        ImGui::TextWrapped(
            "States are badly configured.\n\n"
            "The engine could not find valid game states or scene for this build. "
            "Please ensure you have at least one state registered in the "
            "State Manager and the Start State is set correctly.\n\n"
            "Follow the documentation to configure states properly.");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::Text("Tip: Settings > State Manager in the Editor.");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.25f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.35f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.7f, 0.2f, 0.15f, 1.0f));
        float btnW = 180.0f;
        ImGui::SetCursorPosX((winW - btnW) * 0.5f);
        if (ImGui::Button("Hide Warning (F1)", ImVec2(btnW, 32))) {
            m_ShowStateWarning = false;
        }
        ImGui::PopStyleColor(3);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
        float hintW = ImGui::CalcTextSize("Press F1 to toggle this warning | WASD to move camera").x;
        ImGui::SetCursorPosX((winW - hintW) * 0.5f);
        ImGui::Text("Press F1 to toggle this warning | WASD to move camera");
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2); 
    }
    
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 150, 8), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(140, 0));
        ImGui::SetNextWindowBgAlpha(0.4f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGui::Begin("##FPS", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoInputs);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::Text("%.1fms", 1000.0f / io.Framerate);
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    m_Console->Render(m_Camera.get(), m_RenderContext);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
