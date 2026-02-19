#ifndef EDITOR_LAYER_H
#define EDITOR_LAYER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Scene.h"
#include "Camera.h"
#include "Renderer.h"
#include "Gizmo.h"
#include "Mesh.h"    
#include "Model.h"   
#include "Shader.h"  


struct ImGuiIO;

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
    
    
    bool showDemoWindow = false;
    bool showMetrics = false;
    bool showSceneHierarchy = true;
    bool showInspector = true;
    bool showConsole = true;
    bool showContentBrowser = true;
    
    
    unsigned int viewportTextureID = 0;
    int viewportWidth = 0;
    int viewportHeight = 0;
    
    
    int gizmoOp = 0; 
    bool gizmoLocal = false; 
    
    
    int selectedCube = -1;
    int selectedMesh = -1;
    bool isLightSelected = false; 
    int selectedPointLightIndex = -1;
    
    
    Gizmo gizmo;
    
    
    
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
    
    
    Shader* gizmoProgram = nullptr;

    
    
    float waterHeight = 0.0f;
    float waveSpeed = 1.0f;
    float waveStrength = 0.1f;
    float waterShininess = 32.0f;
    glm::vec3 waterColor = glm::vec3(0.0f, 0.3f, 0.5f); 
    int waveSystem = 0; 

    
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

    
    float globalTilingFactor = 1.0f;
    
    
    bool msaaSkyPass = true;
    bool msaaGeometryPass = true;
    bool msaaTransparencyPass = true;
    
    
    float timeOfDay = 0.0f;
    float timeSpeed = 1.0f;
    bool isTimePaused = false;
    
    
    bool darkTheme = true; 
    bool fixedLayout = false; 

private:
    GLFWwindow* m_Window = nullptr;
    bool m_FirstFrame = true;
    bool m_Initialized = false;
    void DrawMenuBar(Scene& scene);
    void DrawSceneHierarchy(Scene& scene);
    void DrawInspector(Scene& scene);
    void DrawSettings(Camera& camera);
    void DrawViewport(Scene& scene, Camera& camera);
    void DrawContentBrowser();
    void SetupDockLayout();
};

#endif
