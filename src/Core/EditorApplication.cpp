#include "EditorApplication.h"
#include "Editor.h"
#include "Logger.h"
#include "ResourceManager.h"
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
#include "ThreadManager.h"
#include "../Tools/Profiler/Profiler.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Scene/SceneManager.h"
#include "../Scene/ScriptCompiler.h"
#include "../Scene/BehaviorRegistry.h"


static Camera s_GameCam(800, 600, glm::vec3(0.0f));
static bool s_GameCamInitialized = false;

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
    
    SceneManager::Get().SetActiveScene(m_Scene.get());
    SceneManager::Get().SetMainCamera(m_Camera.get());
    
    ResourceManager::LoadShader("gizmo", "../shaders/editor/gizmo.vert", "../shaders/editor/gizmo.frag");
    ResourceManager::LoadShader("skeletal", "../shaders/passes/geometry/skeletal.vert", "../shaders/passes/geometry/skeletal.frag");

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
    } else {
        LoadGlobalSettings();
    }

    return true;
}

void EditorApplication::SaveGlobalSettings() {
    nlohmann::json settings;
    settings["last_project"] = m_ProjectRoot;
    settings["dark_theme"] = m_EditorLayer->darkTheme;
    settings["multithreading"] = ThreadManager::IsEnabled();
    settings["auto_save"] = m_EditorLayer->autoSave;

    std::ofstream file("editor_settings.json");
    if (file.is_open()) {
        file << settings.dump(4);
    }
}

void EditorApplication::LoadGlobalSettings() {
    if (!std::filesystem::exists("editor_settings.json")) return;

    try {
        std::ifstream file("editor_settings.json");
        nlohmann::json settings = nlohmann::json::parse(file);

        if (settings.contains("dark_theme")) {
            m_EditorLayer->darkTheme = settings["dark_theme"].get<bool>();
            if (m_EditorLayer->darkTheme) m_EditorLayer->ApplyDarkTheme();
            else m_EditorLayer->ApplyLightTheme();
        }

        if (settings.contains("multithreading")) {
            ThreadManager::SetEnabled(settings["multithreading"].get<bool>());
        }

        if (settings.contains("auto_save")) {
            m_EditorLayer->autoSave = settings["auto_save"].get<bool>();
        }

        if (settings.contains("last_project")) {
            std::string lastProj = settings["last_project"].get<std::string>();
            if (!lastProj.empty() && std::filesystem::exists(lastProj)) {
                OpenProject(lastProj);
            }
        }
    } catch (...) {}
}

void EditorApplication::SaveProject(bool silent) {
    if (m_ProjectRoot.empty()) return;
    
    nlohmann::json config;
    std::filesystem::path currentScenePath = m_Scene->GetFilepath();
    
    if (!currentScenePath.empty()) {
        if (currentScenePath.filename().string() == "calcium3d_playmode_backup.scene") {
            
            config["last_scene"] = "Scenes/main.scene";
        } else {
            std::filesystem::path projRoot(m_ProjectRoot);
            std::string relPath = std::filesystem::relative(currentScenePath, projRoot).string();
            config["last_scene"] = relPath;
        }
    } else {
        config["last_scene"] = "";
    }
    
    config["start_state"] = (int)Application::Get().m_StartGameState;
    
    
    config["camera"]["position"] = {m_Camera->Position.x, m_Camera->Position.y, m_Camera->Position.z};
    config["camera"]["yaw"] = m_Camera->yaw;
    config["camera"]["pitch"] = m_Camera->pitch;

    
    config["ui"]["showSceneHierarchy"] = m_EditorLayer->showSceneHierarchy;
    config["ui"]["showInspector"] = m_EditorLayer->showInspector;
    config["ui"]["showConsole"] = m_EditorLayer->showConsole;
    config["ui"]["showContentBrowser"] = m_EditorLayer->showContentBrowser;

    config["graphics"]["msaaSamples"] = m_EditorLayer->msaaSamples;
    config["graphics"]["msaaSkyPass"] = m_EditorLayer->msaaSkyPass;
    config["graphics"]["msaaGeometryPass"] = m_EditorLayer->msaaGeometryPass;
    config["graphics"]["msaaTransparencyPass"] = m_EditorLayer->msaaTransparencyPass;
    config["graphics"]["reflectionMode"] = m_EditorLayer->reflectionMode;
    config["graphics"]["ssrUseCubemapFallback"] = m_EditorLayer->ssrUseCubemapFallback;
    config["graphics"]["ssrGeometry"] = m_EditorLayer->ssrGeometry;
    config["graphics"]["ssrTransparency"] = m_EditorLayer->ssrTransparency;
    config["graphics"]["ssrAll"] = m_EditorLayer->ssrAll;
    config["graphics"]["ssrResolution"] = m_EditorLayer->ssrResolution;
    config["graphics"]["ssrMaxSteps"] = m_EditorLayer->ssrMaxSteps;
    config["graphics"]["ssrMaxDistance"] = m_EditorLayer->ssrMaxDistance;
    config["graphics"]["ssrThickness"] = m_EditorLayer->ssrThickness;
    config["graphics"]["ssrRenderDistance"] = m_EditorLayer->ssrRenderDistance;
    config["graphics"]["ssrFadeStart"] = m_EditorLayer->ssrFadeStart;
    
    
    config["environment"]["showSkybox"] = m_EditorLayer->showSkybox;
    config["environment"]["showGradientSky"] = m_EditorLayer->showGradientSky;
    config["environment"]["showWater"] = m_EditorLayer->showWater;
    config["environment"]["showClouds"] = m_EditorLayer->showClouds;
    config["environment"]["cloudMode"] = m_EditorLayer->cloudMode;
    config["environment"]["waterHeight"] = m_EditorLayer->waterHeight;
    config["environment"]["cloudHeight"] = m_EditorLayer->cloud2dHeight;
    config["environment"]["cloudDensity"] = m_EditorLayer->cloudDensity;
    config["environment"]["cloudCover"] = m_EditorLayer->cloudCover;
    config["environment"]["cloudColor"] = {m_EditorLayer->cloudColor.x, m_EditorLayer->cloudColor.y, m_EditorLayer->cloudColor.z};
    config["environment"]["cloudSpeed"] = m_EditorLayer->cloudSpeed;
    config["environment"]["cloudTiling"] = m_EditorLayer->cloudTiling;
    config["environment"]["cloudSize"] = m_EditorLayer->cloudSize;
    config["environment"]["cloudRandomness"] = m_EditorLayer->cloudRandomness;
    
    config["environment"]["waveSpeed"] = m_EditorLayer->waveSpeed;
    config["environment"]["waveStrength"] = m_EditorLayer->waveStrength;
    config["environment"]["waterColor"] = {m_EditorLayer->waterColor.x, m_EditorLayer->waterColor.y, m_EditorLayer->waterColor.z};
    config["environment"]["timeOfDay"] = m_EditorLayer->timeOfDay;
    
    config["environment"]["sunEnabled"] = m_EditorLayer->sunEnabled;
    config["environment"]["sunIntensity"] = m_EditorLayer->sunIntensity;
    config["environment"]["sunColor"] = {m_EditorLayer->sunColor.x, m_EditorLayer->sunColor.y, m_EditorLayer->sunColor.z, m_EditorLayer->sunColor.w};
    config["environment"]["sunBloom"] = m_EditorLayer->sunBloom;
    
    config["environment"]["moonEnabled"] = m_EditorLayer->moonEnabled;
    config["environment"]["moonIntensity"] = m_EditorLayer->moonIntensity;
    config["environment"]["moonColor"] = {m_EditorLayer->moonColor.x, m_EditorLayer->moonColor.y, m_EditorLayer->moonColor.z, m_EditorLayer->moonColor.w};
    config["environment"]["moonBloom"] = m_EditorLayer->moonBloom;

    config["environment"]["enableShadows"] = m_EditorLayer->enableShadows;
    config["environment"]["enablePointShadows"] = m_EditorLayer->enablePointShadows;
    config["environment"]["shadowBias"] = m_EditorLayer->shadowBias;
    config["environment"]["globalTilingFactor"] = m_EditorLayer->globalTilingFactor;

    std::string configPath = m_ProjectRoot + "/project.c3dproj";
    std::ofstream file(configPath);
    if (file.is_open()) {
        file << config.dump(4);
        if (!silent)
            Logger::AddLog("Saved Project Configuration: %s", configPath.c_str());
        SaveGlobalSettings();
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
    SceneManager::Get().SetActiveScene(m_Scene.get()); 
    SceneManager::Get().SetMainCamera(m_Camera.get());
    m_Scene->SetProjectRoot(path);
    m_EditorLayer->selectedCube = -1;
    m_EditorLayer->selectedMesh = -1;
    m_EditorLayer->isLightSelected = false;
    m_EditorLayer->selectedPointLightIndex = -1;
    m_EditorLayer->selectedUIElement = -1;

    Logger::AddLog("Opened Project: %s", path.c_str());

    std::string uiLayoutPath = path + "/ui_layout.json";
    if (std::filesystem::exists(uiLayoutPath)) {
        UICreationEngine::LoadLayout(uiLayoutPath);
        Logger::AddLog("Restored UI layout from %s", uiLayoutPath.c_str());
    }
    
    
    
    
    std::string engineRoot = "";
    
    
    std::filesystem::path searchPath = std::filesystem::current_path();
    for (int i = 0; i < 5; ++i) { 
        if (std::filesystem::exists(searchPath / "src" / "Core" / "Application.cpp")) {
            engineRoot = searchPath.string();
            break;
        }
        if (!searchPath.has_parent_path()) break;
        searchPath = searchPath.parent_path();
    }

    if (engineRoot.empty()) {
        engineRoot = "/home/light/Documents/C3D/Calcium3D"; 
        Logger::AddLog("[ScriptCompiler] Engine root not found, using fallback: %s", engineRoot.c_str());
    } else {
        Logger::AddLog("[ScriptCompiler] Engine root detected: %s", engineRoot.c_str());
    }

    std::string scriptsDir = path + "/Scripts";
    if (std::filesystem::exists(scriptsDir)) {
        Logger::AddLog("[ScriptCompiler] Auto-compiling scripts in: %s", scriptsDir.c_str());
        for (auto& entry : std::filesystem::directory_iterator(scriptsDir)) {
            if (entry.path().extension() == ".cpp") {
                std::string cppPath = entry.path().string();
                Logger::AddLog("[ScriptCompiler] Compiling: %s", entry.path().filename().string().c_str());
                ScriptCompiler::CompileAndLoad(cppPath, engineRoot);
            }
        }
        Logger::AddLog("[ScriptCompiler] Pre-compile complete. Loading scene now.");
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

            
            if (config.contains("camera")) {
                auto& cam = config["camera"];
                if (cam.contains("position")) {
                    m_Camera->Position = glm::vec3(cam["position"][0], cam["position"][1], cam["position"][2]);
                }
                if (cam.contains("yaw")) m_Camera->yaw = cam["yaw"].get<float>();
                if (cam.contains("pitch")) m_Camera->pitch = cam["pitch"].get<float>();
            }

            
            if (config.contains("ui")) {
                auto& ui = config["ui"];
                if (ui.contains("showSceneHierarchy")) m_EditorLayer->showSceneHierarchy = ui["showSceneHierarchy"].get<bool>();
                if (ui.contains("showInspector")) m_EditorLayer->showInspector = ui["showInspector"].get<bool>();
                if (ui.contains("showConsole")) m_EditorLayer->showConsole = ui["showConsole"].get<bool>();
                if (ui.contains("showContentBrowser")) m_EditorLayer->showContentBrowser = ui["showContentBrowser"].get<bool>();
            }

            if (config.contains("graphics")) {
                auto& g = config["graphics"];
                if (g.contains("msaaSamples")) m_EditorLayer->msaaSamples = g["msaaSamples"].get<int>();
                if (g.contains("msaaSkyPass")) m_EditorLayer->msaaSkyPass = g["msaaSkyPass"].get<bool>();
                if (g.contains("msaaGeometryPass")) m_EditorLayer->msaaGeometryPass = g["msaaGeometryPass"].get<bool>();
                if (g.contains("msaaTransparencyPass")) m_EditorLayer->msaaTransparencyPass = g["msaaTransparencyPass"].get<bool>();
                if (g.contains("reflectionMode")) m_EditorLayer->reflectionMode = g["reflectionMode"].get<int>();
                if (g.contains("ssrUseCubemapFallback")) m_EditorLayer->ssrUseCubemapFallback = g["ssrUseCubemapFallback"].get<bool>();
                if (g.contains("ssrGeometry")) m_EditorLayer->ssrGeometry = g["ssrGeometry"].get<bool>();
                if (g.contains("ssrTransparency")) m_EditorLayer->ssrTransparency = g["ssrTransparency"].get<bool>();
                if (g.contains("ssrAll")) m_EditorLayer->ssrAll = g["ssrAll"].get<bool>();
                if (g.contains("ssrResolution")) m_EditorLayer->ssrResolution = g["ssrResolution"].get<float>();
                if (g.contains("ssrMaxSteps")) m_EditorLayer->ssrMaxSteps = g["ssrMaxSteps"].get<int>();
                if (g.contains("ssrMaxDistance")) m_EditorLayer->ssrMaxDistance = g["ssrMaxDistance"].get<float>();
                if (g.contains("ssrThickness")) m_EditorLayer->ssrThickness = g["ssrThickness"].get<float>();
                if (g.contains("ssrRenderDistance")) m_EditorLayer->ssrRenderDistance = g["ssrRenderDistance"].get<float>();
                if (g.contains("ssrFadeStart")) m_EditorLayer->ssrFadeStart = g["ssrFadeStart"].get<float>();
            }

            if (config.contains("environment")) {
                auto& env = config["environment"];
                if (env.contains("showSkybox")) m_EditorLayer->showSkybox = env["showSkybox"].get<bool>();
                if (env.contains("showGradientSky")) m_EditorLayer->showGradientSky = env["showGradientSky"].get<bool>();
                if (env.contains("showWater")) m_EditorLayer->showWater = env["showWater"].get<bool>();
                if (env.contains("showClouds")) m_EditorLayer->showClouds = env["showClouds"].get<bool>();
                if (env.contains("cloudMode")) m_EditorLayer->cloudMode = env["cloudMode"].get<int>();
                if (env.contains("waterHeight")) m_EditorLayer->waterHeight = env["waterHeight"].get<float>();
                if (env.contains("cloudHeight")) m_EditorLayer->cloud2dHeight = env["cloudHeight"].get<float>();
                if (env.contains("cloudDensity")) m_EditorLayer->cloudDensity = env["cloudDensity"].get<float>();
                if (env.contains("cloudCover")) m_EditorLayer->cloudCover = env["cloudCover"].get<float>();
                if (env.contains("cloudColor")) m_EditorLayer->cloudColor = glm::vec3(env["cloudColor"][0], env["cloudColor"][1], env["cloudColor"][2]);
                if (env.contains("cloudSpeed")) m_EditorLayer->cloudSpeed = env["cloudSpeed"].get<float>();
                if (env.contains("cloudTiling")) m_EditorLayer->cloudTiling = env["cloudTiling"].get<float>();
                if (env.contains("cloudSize")) m_EditorLayer->cloudSize = env["cloudSize"].get<float>();
                if (env.contains("cloudRandomness")) m_EditorLayer->cloudRandomness = env["cloudRandomness"].get<float>();

                if (env.contains("waveSpeed")) m_EditorLayer->waveSpeed = env["waveSpeed"].get<float>();
                if (env.contains("waveStrength")) m_EditorLayer->waveStrength = env["waveStrength"].get<float>();
                if (env.contains("waterColor")) m_EditorLayer->waterColor = glm::vec3(env["waterColor"][0], env["waterColor"][1], env["waterColor"][2]);
                if (env.contains("timeOfDay")) m_EditorLayer->timeOfDay = env["timeOfDay"].get<float>();
                
                if (env.contains("sunEnabled")) m_EditorLayer->sunEnabled = env["sunEnabled"].get<bool>();
                if (env.contains("sunIntensity")) m_EditorLayer->sunIntensity = env["sunIntensity"].get<float>();
                if (env.contains("sunColor")) m_EditorLayer->sunColor = glm::vec4(env["sunColor"][0], env["sunColor"][1], env["sunColor"][2], env["sunColor"][3]);
                if (env.contains("sunBloom")) m_EditorLayer->sunBloom = env["sunBloom"].get<float>();

                if (env.contains("moonEnabled")) m_EditorLayer->moonEnabled = env["moonEnabled"].get<bool>();
                if (env.contains("moonIntensity")) m_EditorLayer->moonIntensity = env["moonIntensity"].get<float>();
                if (env.contains("moonColor")) m_EditorLayer->moonColor = glm::vec4(env["moonColor"][0], env["moonColor"][1], env["moonColor"][2], env["moonColor"][3]);
                if (env.contains("moonBloom")) m_EditorLayer->moonBloom = env["moonBloom"].get<float>();

                if (env.contains("enableShadows")) m_EditorLayer->enableShadows = env["enableShadows"].get<bool>();
                if (env.contains("enablePointShadows")) m_EditorLayer->enablePointShadows = env["enablePointShadows"].get<bool>();
                if (env.contains("shadowBias")) m_EditorLayer->shadowBias = env["shadowBias"].get<float>();
                if (env.contains("globalTilingFactor")) m_EditorLayer->globalTilingFactor = env["globalTilingFactor"].get<float>();
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
    s_GameCamInitialized = false; 
    
    
    m_PlayModeSceneBackup = "/tmp/calcium3d_playmode_backup.scene";
    m_Scene->Save(m_PlayModeSceneBackup);
    
    
    int objCount = (int)m_Scene->GetObjects().size();
    for (int i = 0; i < objCount; i++) {
        auto& obj = m_Scene->GetObjects()[i];
        if (obj.hasAudio && obj.audio.playOnAwake && !obj.audio.filePath.empty()) {
            obj.audio.playing = true;
        }
        for (auto& script : obj.behaviors) {
            if (script) {
                script->gameObject = &m_Scene->GetObjects()[i]; 
                script->OnStart();
            }
        }
    }

    
    Logger::AddLog("Entered Play Mode");
}

void EditorApplication::ExitPlayMode() {
    if (!m_PlayMode) return;
    m_PlayMode = false;
    Editor::isEditMode = true;
    s_GameCamInitialized = false; 
    
    
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
    
    Logger::AddLog("Exited Play Mode — Scene Restored");
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

    SceneManager::Get().Update(deltaTime);

    {
        PROFILE_SCOPE("Camera");
        
        
        bool gameCamActive = !Editor::isEditMode && m_Scene->GetGameCameraIndex() != -1;
        if ((Editor::isEditMode || m_EditorLayer->m_MasterControl) && !gameCamActive) {
             m_Camera->Inputs(m_Window, deltaTime, m_EditorLayer->m_MasterControl);
        }
    }
    
    if (!Editor::isEditMode) {
        PROFILE_SCOPE("Scripts+Physics");
        
        int gameCamIdx = m_Scene->GetGameCameraIndex();

        
        if (gameCamIdx != -1) {
            auto& objs = m_Scene->GetObjects();
            if (gameCamIdx >= 0 && gameCamIdx < (int)objs.size() && objs[gameCamIdx].hasCamera) {
                auto& obj = objs[gameCamIdx];
                
                if (!s_GameCamInitialized) {
                    s_GameCam.Position = obj.position;
                    s_GameCam.Orientation = obj.rotation * glm::vec3(0, 0, -1);
                    s_GameCam.Up = obj.rotation * glm::vec3(0, 1, 0);
                    s_GameCam.FOV = obj.camera.fov;
                    s_GameCam.nearPlane = obj.camera.nearPlane;
                    s_GameCam.farPlane = obj.camera.farPlane;
                    s_GameCamInitialized = true;
                }
                s_GameCam.width = m_ViewportWidth;
                s_GameCam.height = m_ViewportHeight;
                m_RenderContext.camera = &s_GameCam;
            }
        } else {
            m_RenderContext.camera = m_Camera.get();
        }

        m_Scene->Update(deltaTime, (float)glfwGetTime());

        
        
        
        if (gameCamIdx != -1) {
            s_GameCam.Position    = m_Camera->Position;
            s_GameCam.Orientation = m_Camera->Orientation;
            s_GameCam.Up          = m_Camera->Up;
        }
    }

    
    if (m_EditorLayer->m_FocusRequested && Editor::isEditMode) {
        m_EditorLayer->m_FocusRequested = false;
        glm::vec3 target = m_EditorLayer->m_FocusTarget;
        
        glm::vec3 offset = glm::vec3(0.0f, 2.5f, 5.0f);
        m_Camera->Position    = target + offset;
        
        glm::vec3 dir = glm::normalize(target - m_Camera->Position);
        m_Camera->Orientation = dir;
        
        m_Camera->pitch = glm::degrees(asin(dir.y));
        m_Camera->yaw   = glm::degrees(atan2(dir.z, dir.x));
    }

    m_ShowSkybox = m_EditorLayer->showSkybox;
    m_ShowGradientSky = m_EditorLayer->showGradientSky;
    m_ShowClouds = m_EditorLayer->showClouds;

    if (m_MSAASamples != m_EditorLayer->msaaSamples) {
        CreateMSAAFramebuffer(m_EditorLayer->msaaSamples);
    }
}

void EditorApplication::OnRender()
{
    
    GpuProfiler::Get().SetEnabled(Profiler::Get().IsEnabled());
    GpuProfiler::Get().SetPaused(Profiler::Get().IsPaused());
    Profiler::Get().BeginFrame();
    GpuProfiler::Get().BeginFrame();   
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
    if (m_PlayMode) {
        int gameCamIdx = m_Scene->GetGameCameraIndex();
        if (gameCamIdx != -1) {
            auto& objs = m_Scene->GetObjects();
            if (gameCamIdx >= 0 && gameCamIdx < (int)objs.size() && objs[gameCamIdx].hasCamera) {
                
                m_RenderContext.camera = &s_GameCam;
                
                objs[gameCamIdx].isActive = false;
            }
        }
    }
    
    m_RenderContext.scene = m_Scene.get();
    m_RenderContext.cloud2d = m_Cloud2D.get();
    m_RenderContext.volCloud = m_VolumetricCloud.get();
    
    m_RenderContext.time = glfwGetTime();
    
    if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
        m_RenderContext.mainFBO = m_MSAAFBO;
    } else {
        m_RenderContext.mainFBO = m_ViewportFBO;
    }
    
    m_RenderContext.showClouds = m_ShowClouds;
    m_RenderContext.cloudMode = m_EditorLayer->cloudMode;
    m_RenderContext.cloudHeight = m_EditorLayer->cloud2dHeight;
    m_RenderContext.cloudDensity = m_EditorLayer->cloudDensity; 
    m_RenderContext.cloudCover = m_EditorLayer->cloudCover;
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

    m_RenderContext.reflectionMode         = m_EditorLayer->reflectionMode;
    m_RenderContext.ssrUseCubemapFallback  = m_EditorLayer->ssrUseCubemapFallback;
    m_RenderContext.ssrGeometry = m_EditorLayer->ssrGeometry;
    m_RenderContext.ssrTransparency = m_EditorLayer->ssrTransparency;
    m_RenderContext.ssrAll = m_EditorLayer->ssrAll;
    m_RenderContext.ssrResolution = m_EditorLayer->ssrResolution;
    m_RenderContext.ssrMaxSteps = m_EditorLayer->ssrMaxSteps;
    m_RenderContext.ssrMaxDistance = m_EditorLayer->ssrMaxDistance;
    m_RenderContext.ssrThickness = m_EditorLayer->ssrThickness;
    m_RenderContext.ssrRenderDistance = m_EditorLayer->ssrRenderDistance;
    m_RenderContext.ssrFadeStart = m_EditorLayer->ssrFadeStart;

    ProcessSceneCameras();

    if (m_EditorLayer->msaaSamples > 0 && m_MSAAFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_MSAAFBO);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ViewportFBO);
    }
    glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

    m_RenderContext.renderEditorObjects = true;
    {
        PROFILE_SCOPE("RenderPipeline");
        m_RenderPipeline->Execute(m_RenderContext);
    }

    
    if (m_PlayMode) {
        int gameCamIdx = m_Scene->GetGameCameraIndex();
        if (gameCamIdx != -1) {
            auto& objs = m_Scene->GetObjects();
            if (gameCamIdx >= 0 && gameCamIdx < (int)objs.size()) {
                objs[gameCamIdx].isActive = true;
            }
        }
    }

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

    {
        PROFILE_SCOPE("UI");
        m_EditorLayer->Render(*m_Scene, *m_Camera, deltaTime);
    }
    
    m_EditorLayer->RenderTransitions();
}

void EditorApplication::PostRender() {
    m_EditorLayer->End();
    GpuProfiler::Get().EndFrame();   
    
    static double s_LastTime = glfwGetTime();
    double now = glfwGetTime();
    float dt = (float)(now - s_LastTime);
    s_LastTime = now;
    Profiler::Get().EndFrame(dt * 1000.0f);
}

void EditorApplication::Shutdown() {
    SaveProject();
    m_EditorLayer->Shutdown();
    Application::Shutdown();
}

void EditorApplication::CreateMSAAFramebuffer(int samples) {
    if (m_MSAAFBO != 0) {
        glDeleteFramebuffers(1, &m_MSAAFBO);
        glDeleteTextures(1, &m_MSAAColorBuffer);
        glDeleteTextures(1, &m_MSAANormalBuffer);
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

        glGenTextures(1, &m_MSAANormalBuffer);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_MSAANormalBuffer);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, m_MSAASamples, GL_RGBA16F, m_ViewportWidth, m_ViewportHeight, GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, m_MSAANormalBuffer, 0);

        glGenRenderbuffers(1, &m_MSAARBO);
        glBindRenderbuffer(GL_RENDERBUFFER, m_MSAARBO);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_MSAASamples, GL_DEPTH24_STENCIL8, m_ViewportWidth, m_ViewportHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_MSAARBO);

        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void EditorApplication::CreateViewportFramebuffer(int width, int height) {
    
    if (m_ViewportFBO != 0) {
        glDeleteFramebuffers(1, &m_ViewportFBO);
        glDeleteTextures(1, &m_ViewportTexture);
        glDeleteTextures(1, &m_ViewportNormalTexture);
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

    glGenTextures(1, &m_ViewportNormalTexture);
    glBindTexture(GL_TEXTURE_2D, m_ViewportNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_ViewportNormalTexture, 0);

    glGenRenderbuffers(1, &m_ViewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_ViewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_ViewportRBO);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

