#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <thread>
#include <atomic>
#include <mutex>

#include "Scene.h"
#include "Camera.h"
#include "Renderer.h"
#include "Gizmo.h"
#include "Mesh.h"    
#include "Model.h"   
#include "Shader.h"  

#include "../Tools/Profiler/Profiler.h"
#include "../Tools/Profiler/ProfilerUI.h"

class EditorLayer {
public:
    EditorLayer();
    ~EditorLayer();

    void Init(GLFWwindow* window);
    void Shutdown();

    void Begin();
    void End();

    void Render(Scene& scene, Camera& camera, float dt);
    void RenderOverlay(Scene& scene, Camera& camera);
    void RenderTransitions();
    
    
    bool showDemoWindow = false;
    bool showMetrics = false;
    bool showSceneHierarchy = true;
    bool showInspector = true;
    bool showConsole = true;
    bool showContentBrowser = true;

    bool showProfiler = false;
    bool showProjectSettings = false;
    
    
    unsigned int viewportTextureID = 0;
    int viewportWidth = 0;
    int viewportHeight = 0;
    
    
    int gizmoOp = 0; 
    bool gizmoLocal = false; 
    Gizmo gizmo;
    
    


    
    int selectedCube = -1;
    std::set<int> selectedObjects;
    int lastSelectedObject = -1;
    std::vector<GameObject> m_ClipboardNodes;
    int selectedMesh = -1;
    std::string m_EditingShaderPath;
    char m_ShaderEditorBuffer[65536] = "";
    bool isLightSelected = false; 
    int selectedPointLightIndex = -1;
    
    
    Model* plane = nullptr;
    Mesh* light = nullptr;
    Mesh* sun = nullptr;
    glm::vec3 lightPos = glm::vec3(0.0f);
    glm::vec3 sunPos = glm::vec3(0.0f);
    glm::vec4 sunColor = glm::vec4(1.0f);
    float sunIntensity = 1.0f;
    bool sunEnabled = true;
    
    glm::vec3 moonPos = glm::vec3(0.0f); 
    glm::vec4 moonColor = glm::vec4(0.8f, 0.8f, 0.9f, 1.0f);
    float moonIntensity = 0.5f;
    bool moonEnabled = true;
    
    bool showPlane = true;
    bool showLightSource = true;
    bool showSkybox = true;
    bool showGradientSky = false;
    bool showClouds = false;
    bool showWater = false;
    
    
    bool wireframe = false;
    bool vsync = true;
    int msaaSamples = 4;
    bool msaaSkyPass = true;
    bool msaaGeometryPass = true;
    bool msaaTransparencyPass = true;
    float globalTilingFactor = 1.0f;
    
    
    bool enableShadows = true;
    bool enablePointShadows = false;
    float shadowBias = 0.005f;

    
    
    int  reflectionMode = 0;           
    bool ssrUseCubemapFallback = true;
    bool ssrGeometry = false;
    bool ssrTransparency = false;
    bool ssrAll = false;
    float ssrResolution = 0.5f;
    int ssrMaxSteps = 30;
    float ssrMaxDistance = 50.0f;
    float ssrThickness = 2.0f;
    float ssrRenderDistance = 100.0f;
    float ssrFadeStart = 60.0f;
    
    float timeOfDay = 0.0f;
    float timeSpeed = 1.0f;
    bool isTimePaused = false;
    
    
    float waterHeight = 0.0f;
    float waveSpeed = 1.0f;
    float waveStrength = 0.1f;
    float waterShininess = 32.0f;
    int waveSystem = 0;
    glm::vec3 waterColor = glm::vec3(0.0f, 0.3f, 0.5f); 
    
    
    Shader* gizmoProgram = nullptr;
    
    
    int cloudMode = 0; 
    float cloud2dHeight = 50.0f;
    glm::vec3 cloudColor = glm::vec3(1.0f);
    float cloudCover = 0.5f;
    float cloudSpeed = 0.1f;
    float cloudTiling = 1.0f;
    float cloudDensity = 0.5f;
    float cloudSize = 1.0f;
    float cloudRandomness = 0.5f;

    float volCloudDensity = 0.5f;
    float volCloudStepSize = 0.5f;
    float volCloudCover = 0.5f;
    float volCloudSpeed = 0.05f;
    float volCloudDetail = 2.0f;
    int volCloudQuality = 0;

    float sunBloom = 0.03f;
    float moonBloom = 0.02f;

    
    bool darkTheme = true; 
    bool fixedLayout = false; 
    bool autoSave = true;
    bool m_MasterControl = false;

    
    bool m_FocusRequested = false;
    glm::vec3 m_FocusTarget = glm::vec3(0.0f);

    void ApplyDarkTheme();
    void ApplyLightTheme();
    void TriggerAutoSave(Scene& scene);

    
    void SetContentPath(const std::string& path) { 
        m_ProjectRoot = path;
        m_CurrentContentPath = path; 
        
        
        std::filesystem::path targetShadersDir = std::filesystem::path(path) / "Shaders";
        
        std::string sourceShaders = "";
        if (std::filesystem::exists("shaders")) sourceShaders = "shaders";
        else if (std::filesystem::exists("../shaders")) sourceShaders = "../shaders";
        else if (std::filesystem::exists("../../shaders")) sourceShaders = "../../shaders";
        
        if (!sourceShaders.empty()) {
            try {
                std::filesystem::copy(sourceShaders, targetShadersDir, std::filesystem::copy_options::recursive | std::filesystem::copy_options::skip_existing);
                std::cout << "[EditorLayer] Successfully copied shaders from " << sourceShaders << " to " << targetShadersDir << "\n";
            } catch(const std::exception& e) {
                std::cerr << "[EditorLayer] Failed to copy shaders: " << e.what() << "\n";
            }
        } else {
            std::cerr << "[EditorLayer] Could not locate engine shaders directory to copy!\n";
        }
    }

    int GetPreviewState() const { return m_PreviewState; }

    friend class EditorApplication;

private:
    GLFWwindow* m_Window = nullptr;
    bool m_FirstFrame = true;
    bool m_Initialized = false;

    std::string m_ProjectRoot = "";
    std::string m_CurrentContentPath = "";
    std::string m_SelectedFolderPath = "";
    bool m_GizmoUsingLastFrame = false;
    
    bool uiEditMode = false;
    int selectedUIElement = -1;
    int m_PreviewState = 0;
    std::string m_PlayModeActiveScreen;
    std::string m_SelectedContentFile = ""; 

    void DrawMenuBar(Scene& scene);
    void DrawSceneHierarchy(Scene& scene);
    void DrawInspector(Scene& scene);
    void DrawProjectSettings(Scene& scene);
    void DrawSettings(Scene& scene, Camera& camera);
    void DrawViewport(Scene& scene, Camera& camera);
    void DrawContentBrowser();
    void DrawAnimator(Scene& scene);
    void DrawUIEditor();
    void DrawContentBrowserTree(const std::string& path);
    void DrawContentBrowserGrid();
    void DrawShaderEditor();
    void SetupDockLayout();
    void DrawBuildModal(Scene& scene);

    
    bool m_ShowBuildModal = false;
    bool m_BuildInProgress = false;
    bool m_BuildDone = false;
    bool m_BuildSuccess = false;
    std::atomic<float> m_BuildProgress{0.0f};
    std::string m_BuildStageMsg = "";
    std::mutex m_BuildMsgMutex;
    std::thread m_BuildThread;
    char m_BuildOutputPath[256] = "Build/Linux";
    char m_BuildStartScene[128] = "main.scene";
    int  m_BuildPlatform = 0; 
    bool m_DisableStateWarning = false;
    bool m_EnableMasterControlInBuild = false;
    bool m_EnableHitboxesInBuild = false;
};

#endif
