#include "EditorLayer.h"
#include "PlayMode.h"
#include "ThreadManager.h"
#include "../AudioEngine/AudioEngine.h"
#include "Editor.h"
#include "Logger.h"
#include "ObjectFactory.h"
#include "ResourceManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/Passes/SkyPass.h"
#include "../Physics/PhysicsEngine.h"
#include "../Physics/HitboxGraphics.h"
#include "Core/Application.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "Shader.h"  
#include "BuildManager/BuildManager.h"
#include "Core/EditorApplication.h"
#include "Scene/ScriptCompiler.h"
#include "ModelImport/ModelImporter.h"
#include <imgui_internal.h> 
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include "UI/UIManager.h"
#include "UI/UICreationEngine.h"
#include "UI/Screens/StartScreen.h"
#include "Core/GameState.h"
#include "Scene/BehaviorRegistry.h"
#include "Scene/Behavior.h"
#include <functional>
#include <unordered_map>
#include <algorithm>

EditorLayer::EditorLayer() {
}

EditorLayer::~EditorLayer() {
    Shutdown();
}




static void ApplyProfessionalDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    
    style.WindowPadding     = ImVec2(8, 8);
    style.WindowRounding    = 2.0f;
    style.WindowBorderSize  = 1.0f;
    style.WindowMinSize     = ImVec2(100, 50);
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);

    
    style.FramePadding      = ImVec2(6, 3);
    style.FrameRounding     = 2.0f;
    style.FrameBorderSize   = 0.0f;

    
    style.ItemSpacing       = ImVec2(8, 4);
    style.ItemInnerSpacing  = ImVec2(4, 4);
    style.IndentSpacing     = 18.0f;

    
    style.GrabMinSize       = 10.0f;
    style.GrabRounding      = 2.0f;
    style.ScrollbarSize     = 14.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding       = 2.0f;
    style.PopupRounding     = 2.0f;

    
    ImVec4 bg         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); 
    ImVec4 bgChild    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); 
    ImVec4 bgPopup    = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    ImVec4 border     = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    ImVec4 titleBg    = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    ImVec4 titleActive= ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    ImVec4 tab        = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    ImVec4 tabActive  = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    ImVec4 tabHover   = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    ImVec4 frame      = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    ImVec4 frameHover = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImVec4 frameActive= ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    ImVec4 accent     = ImVec4(0.11f, 0.40f, 0.80f, 1.00f); 
    ImVec4 accentHover= ImVec4(0.15f, 0.50f, 0.90f, 1.00f);
    ImVec4 accentActive=ImVec4(0.08f, 0.35f, 0.70f, 1.00f);
    ImVec4 text       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 textDim    = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 header     = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    ImVec4 headerHover= ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    ImVec4 headerActive=ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    ImVec4 scrollbar  = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    ImVec4 scrollGrab = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    ImVec4 separator  = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    colors[ImGuiCol_Text]                  = text;
    colors[ImGuiCol_TextDisabled]          = textDim;
    colors[ImGuiCol_WindowBg]              = bg;
    colors[ImGuiCol_ChildBg]               = bgChild;
    colors[ImGuiCol_PopupBg]               = bgPopup;
    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg]               = frame;
    colors[ImGuiCol_FrameBgHovered]        = frameHover;
    colors[ImGuiCol_FrameBgActive]         = frameActive;
    colors[ImGuiCol_TitleBg]               = titleBg;
    colors[ImGuiCol_TitleBgActive]         = titleActive;
    colors[ImGuiCol_TitleBgCollapsed]      = titleBg;
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = scrollbar;
    colors[ImGuiCol_ScrollbarGrab]         = scrollGrab;
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark]             = accent;
    colors[ImGuiCol_SliderGrab]            = accent;
    colors[ImGuiCol_SliderGrabActive]      = accentActive;
    colors[ImGuiCol_Button]                = frame;
    colors[ImGuiCol_ButtonHovered]         = frameHover;
    colors[ImGuiCol_ButtonActive]          = accent;
    colors[ImGuiCol_Header]                = header;
    colors[ImGuiCol_HeaderHovered]         = headerHover;
    colors[ImGuiCol_HeaderActive]          = headerActive;
    colors[ImGuiCol_Separator]             = separator;
    colors[ImGuiCol_SeparatorHovered]      = accentHover;
    colors[ImGuiCol_SeparatorActive]       = accent;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]     = accentHover;
    colors[ImGuiCol_ResizeGripActive]      = accent;
    colors[ImGuiCol_Tab]                   = tab;
    colors[ImGuiCol_TabHovered]            = tabHover;
    colors[ImGuiCol_TabSelected]           = tabActive;
    colors[ImGuiCol_TabDimmed]             = tab;
    colors[ImGuiCol_TabDimmedSelected]     = tabActive;
    colors[ImGuiCol_DockingPreview]        = ImVec4(accent.x, accent.y, accent.z, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PlotLines]             = accent;
    colors[ImGuiCol_PlotLinesHovered]      = accentHover;
    colors[ImGuiCol_PlotHistogram]         = accent;
    colors[ImGuiCol_PlotHistogramHovered]  = accentHover;
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = border;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = accent;
    colors[ImGuiCol_NavHighlight]          = accent;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}




static void ApplyProfessionalLightTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowPadding     = ImVec2(8, 8);
    style.WindowRounding    = 2.0f;
    style.WindowBorderSize  = 1.0f;
    style.WindowMinSize     = ImVec2(100, 50);
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);
    style.FramePadding      = ImVec2(6, 3);
    style.FrameRounding     = 2.0f;
    style.FrameBorderSize   = 1.0f;
    style.ItemSpacing       = ImVec2(8, 4);
    style.ItemInnerSpacing  = ImVec2(4, 4);
    style.IndentSpacing     = 18.0f;
    style.GrabMinSize       = 10.0f;
    style.GrabRounding      = 2.0f;
    style.ScrollbarSize     = 14.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding       = 2.0f;
    style.PopupRounding     = 2.0f;

    ImVec4 bg         = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    ImVec4 bgChild    = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    ImVec4 bgPopup    = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    ImVec4 border     = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);
    ImVec4 titleBg    = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    ImVec4 titleActive= ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
    ImVec4 frame      = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 frameHover = ImVec4(0.80f, 0.84f, 0.90f, 1.00f);
    ImVec4 frameActive= ImVec4(0.70f, 0.78f, 0.90f, 1.00f);
    ImVec4 accent     = ImVec4(0.16f, 0.44f, 0.82f, 1.00f);
    ImVec4 accentHover= ImVec4(0.20f, 0.52f, 0.92f, 1.00f);
    ImVec4 accentActive=ImVec4(0.12f, 0.38f, 0.72f, 1.00f);
    ImVec4 text       = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    ImVec4 textDim    = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    ImVec4 header     = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    ImVec4 headerHover= ImVec4(0.78f, 0.82f, 0.90f, 1.00f);
    ImVec4 headerActive=ImVec4(0.70f, 0.78f, 0.88f, 1.00f);

    colors[ImGuiCol_Text]                  = text;
    colors[ImGuiCol_TextDisabled]          = textDim;
    colors[ImGuiCol_WindowBg]              = bg;
    colors[ImGuiCol_ChildBg]               = bgChild;
    colors[ImGuiCol_PopupBg]               = bgPopup;
    colors[ImGuiCol_Border]                = border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg]               = frame;
    colors[ImGuiCol_FrameBgHovered]        = frameHover;
    colors[ImGuiCol_FrameBgActive]         = frameActive;
    colors[ImGuiCol_TitleBg]               = titleBg;
    colors[ImGuiCol_TitleBgActive]         = titleActive;
    colors[ImGuiCol_TitleBgCollapsed]      = titleBg;
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_CheckMark]             = accent;
    colors[ImGuiCol_SliderGrab]            = accent;
    colors[ImGuiCol_SliderGrabActive]      = accentActive;
    colors[ImGuiCol_Button]                = frame;
    colors[ImGuiCol_ButtonHovered]         = frameHover;
    colors[ImGuiCol_ButtonActive]          = accent;
    colors[ImGuiCol_Header]                = header;
    colors[ImGuiCol_HeaderHovered]         = headerHover;
    colors[ImGuiCol_HeaderActive]          = headerActive;
    colors[ImGuiCol_Separator]             = border;
    colors[ImGuiCol_SeparatorHovered]      = accentHover;
    colors[ImGuiCol_SeparatorActive]       = accent;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.70f, 0.70f, 0.70f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered]     = accentHover;
    colors[ImGuiCol_ResizeGripActive]      = accent;
    colors[ImGuiCol_Tab]                   = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.78f, 0.82f, 0.90f, 1.00f);
    colors[ImGuiCol_TabSelected]           = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_TabDimmed]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(accent.x, accent.y, accent.z, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_PlotLines]             = accent;
    colors[ImGuiCol_PlotLinesHovered]      = accentHover;
    colors[ImGuiCol_PlotHistogram]         = accent;
    colors[ImGuiCol_PlotHistogramHovered]  = accentHover;
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = border;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(accent.x, accent.y, accent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = accent;
    colors[ImGuiCol_NavHighlight]          = accent;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

void EditorLayer::Init(GLFWwindow* window) {
    m_Window = window;

    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    

    
    ApplyProfessionalDarkTheme();

    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    m_FirstFrame = true;
    m_Initialized = true;
}

void EditorLayer::Shutdown() {
    if (!m_Initialized) return;

    
    if (m_BuildThread.joinable()) m_BuildThread.join();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorLayer::SetupDockLayout() {
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);
    ImGui::DockBuilderSetNodePos(dockspace_id, vp->Pos);

    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
    ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.18f, nullptr, &dock_main_id);
    ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

    ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left_id);
    ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
    ImGui::DockBuilderDockWindow("Settings", dock_right_id);
    ImGui::DockBuilderDockWindow("Content Browser", dock_bottom_id);
    ImGui::DockBuilderDockWindow("Console", dock_bottom_id);
    ImGui::DockBuilderDockWindow("Viewport", dock_main_id);

    ImGui::DockBuilderFinish(dockspace_id);
}

void EditorLayer::DrawShaderEditor() {
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Shader Editor", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Editing: %s", m_EditingShaderPath.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Save & Compile")) {
            std::ofstream out(m_EditingShaderPath);
            if (out.is_open()) {
                out << m_ShaderEditorBuffer;
                out.close();
                Logger::AddLog("Saved shader: %s", m_EditingShaderPath.c_str());
                
                std::filesystem::path p(m_EditingShaderPath);
                std::string shaderName = p.stem().string();
                std::string dir = p.parent_path().string();
                std::string vertPath = dir + "/" + shaderName + ".vert";
                std::string fragPath = dir + "/" + shaderName + ".frag";
                
                if (std::filesystem::exists(vertPath) && std::filesystem::exists(fragPath)) {
                    ResourceManager::ReloadShader(shaderName, vertPath.c_str(), fragPath.c_str());
                    Logger::AddLog("Hot-reloaded shader program: %s", shaderName.c_str());
                } else {
                    Logger::AddLog("Saved. Add paired .vert/.frag to compile as '%s'.", shaderName.c_str());
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            m_EditingShaderPath = "";
        }
        
        ImGui::Separator();
        
        ImGui::InputTextMultiline("##ShaderCode", m_ShaderEditorBuffer, sizeof(m_ShaderEditorBuffer), 
                                  ImVec2(-FLT_MIN, -FLT_MIN), ImGuiInputTextFlags_AllowTabInput);
        
    }
    ImGui::End();
}

void EditorLayer::Begin() {
#ifndef C3D_RUNTIME
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags host_flags = 0;
    host_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    host_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    host_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    host_flags |= ImGuiWindowFlags_NoBackground; 
    host_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpaceHost", nullptr, host_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
    if (fixedLayout) {
        dockFlags |= ImGuiDockNodeFlags_NoUndocking;
        dockFlags |= ImGuiDockNodeFlags_NoResize;
    }
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockFlags);

    if (m_FirstFrame) {
        SetupDockLayout();
        m_FirstFrame = false;
    }

    ImGui::End(); 
#endif
}

void EditorLayer::End() {
#ifndef C3D_RUNTIME
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
}

void EditorLayer::Render(Scene& scene, Camera& camera, float dt) {
#ifndef C3D_RUNTIME
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);

    DrawMenuBar(scene);
    DrawViewport(scene, camera);
    
    if (showSceneHierarchy) DrawSceneHierarchy(scene);
    if (showInspector) DrawInspector(scene);
    
    if (showConsole) {
        Logger::Draw("Console", &showConsole);
    }
    
    if (showContentBrowser) DrawContentBrowser();
    
    if (!m_EditingShaderPath.empty()) DrawShaderEditor();
    
    DrawSettings(camera);
#endif
}

void EditorLayer::RenderOverlay(Scene& scene, Camera& camera) {
    if (!gizmoProgram) {
        
        
        if (ResourceManager::GetShader("gizmo").ID == 0) {
             ResourceManager::LoadShader("gizmo", "shaders/editor/gizmo.vert", "shaders/editor/gizmo.frag");
        }
        gizmoProgram = &ResourceManager::GetShader("gizmo");
    }

    
    if (selectedCube != -1) {
        auto& objects = scene.GetObjects();
        if (selectedCube < objects.size()) {
            glm::vec3 pos = objects[selectedCube].position;
            gizmo.Draw(*gizmoProgram, camera, pos);
            
            
            
            glm::vec3 newPosition = pos;
            glm::vec3 newRotation = glm::degrees(glm::eulerAngles(objects[selectedCube].rotation));
            glm::vec3 newScale = objects[selectedCube].scale;
            
            
            
            gizmo.HandleMouse(m_Window, camera, pos, newPosition, newRotation, newScale);
            
            if (gizmo.GetMode() == Gizmo::TRANSLATE && newPosition != pos) {
                objects[selectedCube].position = newPosition;
            }
            
             if (gizmo.GetMode() == Gizmo::SCALE && (newScale.x != 0.0f || newScale.y != 0.0f || newScale.z != 0.0f)) {
                 objects[selectedCube].scale += newScale;
             }
        }
    }
}

void EditorLayer::DrawMenuBar(Scene& scene) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project")) {
                auto* edApp = dynamic_cast<EditorApplication*>(&Application::Get());
                if (edApp) edApp->SetState(EditorApplication::AppState::Home);
            }
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                selectedCube = -1;
                selectedMesh = -1;
                isLightSelected = false;
                selectedPointLightIndex = -1;
                Logger::AddLog("Cleared all selections");
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(m_Window, true);
            }
            ImGui::EndMenu();
        }

        
        ImGui::Separator();
        if (Editor::isEditMode) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.2f, 1.0f));
            if (ImGui::Button("▶ Play (F7)")) {
                auto* edApp = dynamic_cast<EditorApplication*>(&Application::Get());
                if (edApp) edApp->EnterPlayMode();
            }
            ImGui::PopStyleColor(3);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("■ Stop (F7)")) {
                auto* edApp = dynamic_cast<EditorApplication*>(&Application::Get());
                if (edApp) edApp->ExitPlayMode();
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::Separator();
        
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.3f, 0.7f, 1.0f));
        bool ctrlSpressed = ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S);
        
        if (ImGui::Button("Save") || ctrlSpressed) {
            std::filesystem::path currentScenePath = Application::Get().GetScene()->GetFilepath();
            if (currentScenePath.empty()) {
                currentScenePath = std::filesystem::path(Application::Get().GetProjectRoot()) / "Scenes" / "main.scene";
                Application::Get().GetScene()->SetFilepath(currentScenePath.string());
            }
            Application::Get().GetScene()->Save(currentScenePath.string());
            
            auto* edApp = dynamic_cast<EditorApplication*>(&Application::Get());
            if (edApp) edApp->SaveProject();
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Separator();
        if (ImGui::MenuItem("Build")) {
            m_ShowBuildModal = true;
            m_BuildInProgress = false;
            m_BuildDone = false;
            m_BuildProgress = 0.0f;
            m_BuildPlatform = 0;
            {
                std::lock_guard<std::mutex> lock(m_BuildMsgMutex);
                m_BuildStageMsg = "Ready. Select a platform and press Build.";
            }
        }

        if (ImGui::BeginMenu("Help")) {
             if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
             if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
             ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Scene Hierarchy", NULL, &showSceneHierarchy);
            ImGui::MenuItem("Inspector", NULL, &showInspector);
            ImGui::MenuItem("Console", NULL, &showConsole);
            ImGui::MenuItem("Settings", NULL, &showMetrics); 
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::BeginMenu("Primitives")) {
                if (ImGui::MenuItem("Cube")) {
                    Mesh cubeMesh = ObjectFactory::createCube();
                    GameObject obj(std::move(cubeMesh), "Cube");
                    obj.meshType = MeshType::Cube;
                    scene.AddObject(std::move(obj));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Cube");
                }
                if (ImGui::MenuItem("Sphere")) {
                    Mesh sphereMesh = ObjectFactory::createSphere(32, 16);
                    GameObject obj(std::move(sphereMesh), "Sphere");
                    obj.shape = ColliderShape::Sphere;
                    obj.meshType = MeshType::Sphere;
                    scene.AddObject(std::move(obj));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Sphere");
                }
                if (ImGui::MenuItem("Empty Object")) {
                    std::vector<Vertex> rawVerts;
                    std::vector<GLuint> rawIndices;
                    Mesh emptyMesh(rawVerts, rawIndices, {});
                    GameObject obj(std::move(emptyMesh), "Empty Object");
                    obj.meshType = MeshType::None;
                    scene.AddObject(std::move(obj));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Empty Object");
                }
                if (ImGui::MenuItem("Plane")) {
                    Mesh planeMesh = ObjectFactory::createPlane();
                    GameObject obj(std::move(planeMesh), "Plane");
                    obj.meshType = MeshType::Plane;
                    scene.AddObject(std::move(obj));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Plane");
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Audio Object")) {
                std::vector<Vertex> rawVerts;
                std::vector<GLuint> rawIndices;
                Mesh emptyMesh(rawVerts, rawIndices, {});
                GameObject obj(std::move(emptyMesh), "Audio Source");
                obj.meshType = MeshType::None;
                obj.hasAudio = true;
                scene.AddObject(std::move(obj));
                selectedCube = (int)scene.GetObjects().size() - 1;
                isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                Logger::AddLog("Created Audio Object");
            }
            
            if (ImGui::BeginMenu("Lights")) {
                if (ImGui::MenuItem("Point Light")) {
                    Scene::PointLight* pl = scene.CreatePointLight();
                    if (pl) {
                        int idx = (int)scene.GetPointLights().size() - 1;
                        selectedPointLightIndex = idx;
                        isLightSelected = false; selectedCube = -1; selectedMesh = -1;
                        Logger::AddLog("Created Point Light %d", idx);
                    }
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Cameras")) {
                if (ImGui::MenuItem("Scene Camera")) {
                    GameObject cam(ObjectFactory::createCameraMesh(), "Camera");
                    cam.hasCamera = true;
                    cam.camera.enabled = true;
                    cam.meshType = MeshType::Camera;
                    scene.AddObject(std::move(cam));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Camera Object");
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("2D Image Object")) {
                GameObject img(ObjectFactory::createPlane(), "Image Object");
                img.hasScreen = true;
                img.screen.enabled = true;
                img.screen.type = ScreenType::Image;
                img.meshType = MeshType::Plane;
                scene.AddObject(std::move(img));
                selectedCube = (int)scene.GetObjects().size() - 1;
                isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                Logger::AddLog("Created 2D Image Object");
            }

            if (ImGui::MenuItem("Video Display Object")) {
                GameObject vid(ObjectFactory::createPlane(), "Video Object");
                vid.hasScreen = true;
                vid.screen.enabled = true;
                vid.screen.type = ScreenType::Video;
                vid.meshType = MeshType::Plane;
                scene.AddObject(std::move(vid));
                selectedCube = (int)scene.GetObjects().size() - 1;
                isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                Logger::AddLog("Created Video Object");
            }
            ImGui::EndMenu();
        }

        
        std::string projectName = Application::Get().GetProjectName();
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(projectName.c_str()).x;
        
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "%s", projectName.c_str());
        
        
        ImGui::SameLine(windowWidth - 300);
        float fps = ImGui::GetIO().Framerate;
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "%.1f FPS (%.2f ms)", fps, 1000.0f / fps);
        
        ImGui::SameLine(windowWidth - 120);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mode: %s", Editor::isEditMode ? "EDITOR" : "PLAY");

        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopupModal("Build Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char outputPath[256] = "Build/Linux";
        static char startScene[128] = "main.scene";
        
        ImGui::InputText("Output Path", outputPath, IM_ARRAYSIZE(outputPath));
        ImGui::InputText("Start Scene", startScene, IM_ARRAYSIZE(startScene));
        
        if (ImGui::Button("Build Now", ImVec2(120, 40))) {
            BuildManager::BuildSettings settings;
            settings.ProjectRoot = Application::Get().GetProjectRoot();
            settings.StartScene = startScene;
            settings.StartGameState = (int)Application::Get().m_StartGameState;
            
            std::filesystem::path p(outputPath);
            if (p.is_relative()) {
                settings.OutputPath = (std::filesystem::path(settings.ProjectRoot) / p).string();
            } else {
                settings.OutputPath = p.string();
            }

            BuildManager::Build(settings);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 40))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    DrawBuildModal(scene);
}


void EditorLayer::DrawSceneHierarchy(Scene& scene) {
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Scene Hierarchy", nullptr, lockFlags);
    
    if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& objects = scene.GetObjects();
        
        auto executeCopy = [&]() {
            m_ClipboardNodes.clear();
            std::unordered_map<int, int> oldToClip;
            std::set<int> treesToCopy;
            for (int rootIdx : selectedObjects) {
                if (rootIdx >= objects.size()) continue;
                bool parentSelected = false;
                int curr = objects[rootIdx].parentIndex;
                while(curr != -1) {
                    if (selectedObjects.count(curr)) { parentSelected = true; break; }
                    curr = (curr < objects.size()) ? objects[curr].parentIndex : -1;
                }
                if (!parentSelected) {
                    std::vector<int> q = { rootIdx };
                    for (size_t k = 0; k < q.size(); ++k) {
                        int c = q[k];
                        treesToCopy.insert(c);
                        for (int j = 0; j < objects.size(); ++j) {
                            if (objects[j].parentIndex == c) q.push_back(j);
                        }
                    }
                }
            }
            
            for (int idx : treesToCopy) {
                m_ClipboardNodes.push_back(objects[idx]);
                oldToClip[idx] = m_ClipboardNodes.size() - 1;
            }
            for (auto& obj : m_ClipboardNodes) {
                if (oldToClip.count(obj.parentIndex)) obj.parentIndex = oldToClip[obj.parentIndex];
                else obj.parentIndex = -1;
            }
            Logger::AddLog("Copied %zu objects", m_ClipboardNodes.size());
        };
        
        auto executePaste = [&](int targetParentIndex) {
            if (!m_ClipboardNodes.empty()) {
                std::unordered_map<int, int> clipToNew;
                selectedObjects.clear();
                
                for(int i = 0; i < m_ClipboardNodes.size(); ++i) {
                    auto& src = m_ClipboardNodes[i];
                    Mesh newMesh(const_cast<std::vector<Vertex>&>(src.mesh.vertices), 
                                 const_cast<std::vector<GLuint>&>(src.mesh.indices), 
                                 src.mesh.textures);
                                 
                    GameObject newObj(std::move(newMesh), src.name);
                    newObj.position = src.position;
                    newObj.rotation = src.rotation;
                    newObj.scale = src.scale;
                    newObj.material = src.material;
                    newObj.shape = src.shape;
                    newObj.collisionRadius = src.collisionRadius;
                    newObj.useGravity = src.useGravity;
                    newObj.isStatic = src.isStatic;
                    newObj.mass = src.mass;
                    newObj.friction = src.friction;
                    newObj.restitution = src.restitution;
                    newObj.enableCollision = src.enableCollision;
                    newObj.centerOfMassOffset = src.centerOfMassOffset;
                    newObj.isFolded = src.isFolded;
                    
                    if (clipToNew.count(src.parentIndex)) newObj.parentIndex = clipToNew[src.parentIndex];
                    else newObj.parentIndex = targetParentIndex;
                    
                    scene.AddObject(std::move(newObj));
                    int newIdx = scene.GetObjects().size() - 1;
                    clipToNew[i] = newIdx;
                    selectedObjects.insert(newIdx);
                    selectedCube = newIdx;
                    lastSelectedObject = newIdx;
                }
                Logger::AddLog("Pasted %zu objects", m_ClipboardNodes.size());
            }
        };
        
        std::function<void(int)> drawNode = [&](int index) {
            std::string name = objects[index].name;
            if (name.empty()) name = "GameObject " + std::to_string(index);
            std::string label = name + "##" + std::to_string(index);
            
            bool hasChildren = false;
            for (size_t i = 0; i < objects.size(); ++i) {
                if (objects[i].parentIndex == index) { hasChildren = true; break; }
            }
            
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (selectedObjects.count(index)) flags |= ImGuiTreeNodeFlags_Selected;
            if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (objects[index].isFolded) flags |= ImGuiTreeNodeFlags_DefaultOpen; 
            
            bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)index, flags, "%s", name.c_str());
            
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) {
                if (ImGui::GetIO().KeyCtrl) {
                    if (selectedObjects.count(index)) selectedObjects.erase(index);
                    else selectedObjects.insert(index);
                } else if (ImGui::GetIO().KeyShift && lastSelectedObject != -1) {
                    int minIdx = std::min(index, lastSelectedObject);
                    int maxIdx = std::max(index, lastSelectedObject);
                    for (int i = minIdx; i <= maxIdx; ++i) {
                        if (i < objects.size() && objects[i].parentIndex == objects[index].parentIndex) {
                            selectedObjects.insert(i);
                        }
                    }
                } else {
                    selectedObjects.clear();
                    selectedObjects.insert(index);
                }

                selectedCube = index; 
                lastSelectedObject = index;
                selectedMesh = -1;
                isLightSelected = false;
                selectedPointLightIndex = -1;
                Logger::AddLog("Selected %s", objects[index].name.c_str());
            } else if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                if (!selectedObjects.count(index)) {
                     selectedObjects.clear();
                     selectedObjects.insert(index);
                     selectedCube = index;
                     lastSelectedObject = index;
                }
            }
            
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Copy")) {
                    executeCopy();
                    ImGui::EndPopup();
                    if (nodeOpen && hasChildren) ImGui::TreePop();
                    return;
                }
                if (ImGui::MenuItem("Paste")) {
                    executePaste(index);
                    ImGui::EndPopup();
                    if (nodeOpen && hasChildren) ImGui::TreePop();
                    return;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Duplicate object tree")) {
                    scene.DuplicateObjectTree(index, objects[index].parentIndex);
                    ImGui::EndPopup();
                    if (nodeOpen && hasChildren) ImGui::TreePop();
                    return; 
                }
                if (ImGui::MenuItem("Delete object tree")) {
                    scene.RemoveObjectTree(index);
                    selectedObjects.clear();
                    selectedCube = -1;
                    lastSelectedObject = -1;
                    ImGui::EndPopup();
                    if (nodeOpen && hasChildren) ImGui::TreePop();
                    return; 
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Create Empty Child")) {
                    std::vector<Vertex> rawVerts;
                    std::vector<GLuint> rawIndices;
                    Mesh groupMesh(rawVerts, rawIndices, {});
                    GameObject child(std::move(groupMesh), "Empty Child");
                    child.parentIndex = index;
                    scene.AddObject(std::move(child));
                }
                if (ImGui::MenuItem("Create Camera Child")) {
                    GameObject child(ObjectFactory::createCameraMesh(), "Camera");
                    child.hasCamera = true;
                    child.camera.enabled = true;
                    child.meshType = MeshType::Camera;
                    child.parentIndex = index;
                    scene.AddObject(std::move(child));
                }
                if (ImGui::MenuItem("Create 2D Image Child")) {
                    GameObject child(ObjectFactory::createPlane(), "Image Object");
                    child.hasScreen = true;
                    child.screen.enabled = true;
                    child.screen.type = ScreenType::Image;
                    child.parentIndex = index;
                    scene.AddObject(std::move(child));
                }
                if (ImGui::MenuItem("Create Video Child")) {
                    GameObject child(ObjectFactory::createPlane(), "Video Object");
                    child.hasScreen = true;
                    child.screen.enabled = true;
                    child.screen.type = ScreenType::Video;
                    child.parentIndex = index;
                    scene.AddObject(std::move(child));
                }
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("SCENE_OBJ", &index, sizeof(int));
                ImGui::Text("Move %s", objects[index].name.c_str());
                ImGui::EndDragDropSource();
            }
            
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_OBJ")) {
                    int payloadIndex = *(const int*)payload->Data;
                    if (payloadIndex != index) {
                        bool isCycle = false;
                        int curr = index;
                        while(curr != -1) {
                            if (curr == payloadIndex) { isCycle = true; break; }
                            curr = objects[curr].parentIndex;
                        }
                        if (!isCycle) objects[payloadIndex].parentIndex = index;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            
            if (nodeOpen) {
                if (hasChildren) {
                    for (int i = 0; i < objects.size(); ++i) {
                        
                        if (i < objects.size() && objects[i].parentIndex == index) drawNode(i);
                    }
                    ImGui::TreePop();
                }
            }
        };

        if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
            selectedObjects.clear();
            for (int i = 0; i < objects.size(); ++i) {
                selectedObjects.insert(i);
            }
        }
        
        if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
            executeCopy();
        }
        
        if (ImGui::IsWindowFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
            executePaste((lastSelectedObject != -1 && lastSelectedObject < objects.size()) ? lastSelectedObject : -1);
        }
        
        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            std::vector<int> toDelete;
            for (int idx : selectedObjects) {
                bool parentSelected = false;
                int curr = (idx < objects.size()) ? objects[idx].parentIndex : -1;
                while(curr != -1) {
                    if (selectedObjects.count(curr)) { parentSelected = true; break; }
                    curr = (curr < objects.size()) ? objects[curr].parentIndex : -1;
                }
                if (!parentSelected && idx < objects.size()) toDelete.push_back(idx);
            }
            std::sort(toDelete.begin(), toDelete.end(), std::greater<int>());
            for (int idx : toDelete) scene.RemoveObjectTree(idx);
            selectedObjects.clear();
            selectedCube = -1;
            lastSelectedObject = -1;
            Logger::AddLog("Deleted %zu objects", toDelete.size());
        }

        for (int i = 0; i < objects.size(); ++i) {
            if (objects[i].parentIndex == -1) {
                drawNode(i);
            }
        }
        
        ImGui::InvisibleButton("DropRoot", ImVec2(ImGui::GetContentRegionAvail().x, 30));
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_OBJ")) {
                int payloadIndex = *(const int*)payload->Data;
                objects[payloadIndex].parentIndex = -1;
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextWindow("HierarchyPopup", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::MenuItem("Create Empty Object")) {
                std::vector<Vertex> rawVerts;
                std::vector<GLuint> rawIndices;
                Mesh groupMesh(rawVerts, rawIndices, {});
                GameObject obj(std::move(groupMesh), "Empty Object");
                scene.AddObject(std::move(obj));
            }
            if (ImGui::MenuItem("Create Camera")) {
                GameObject cam(ObjectFactory::createCameraMesh(), "Camera");
                cam.hasCamera = true;
                cam.camera.enabled = true;
                cam.meshType = MeshType::Camera;
                scene.AddObject(std::move(cam));
            }
            if (ImGui::MenuItem("Create 2D Image Object")) {
                GameObject img(ObjectFactory::createPlane(), "Image Object");
                img.hasScreen = true;
                img.screen.enabled = true;
                img.screen.type = ScreenType::Image;
                scene.AddObject(std::move(img));
            }
            if (ImGui::MenuItem("Create Video Object")) {
                GameObject vid(ObjectFactory::createPlane(), "Video Object");
                vid.hasScreen = true;
                vid.screen.enabled = true;
                vid.screen.type = ScreenType::Video;
                scene.AddObject(std::move(vid));
            }
            ImGui::EndPopup();
        }
    }
    
    if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        auto& pointLights = scene.GetPointLights();
        for (int i = 0; i < pointLights.size(); ++i) {
            std::string label = "Point Light " + std::to_string(i);
            bool isSelected = (selectedPointLightIndex == i);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                selectedCube = -1;
                selectedMesh = -1;
                isLightSelected = false;
                selectedPointLightIndex = i;
                Logger::AddLog("Selected Point Light %d", i);
            }
        }
    }

    ImGui::End();
}

void EditorLayer::DrawInspector(Scene& scene) {
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Inspector", nullptr, lockFlags);
    
    if (selectedCube != -1) {
        auto& objects = scene.GetObjects();
        if (selectedCube < objects.size()) {
            GameObject& obj = objects[selectedCube];
            ImGui::Text("Type: GameObject");
            ImGui::InputText("Name", &obj.name[0], obj.name.capacity() + 1); 
            ImGui::Checkbox("Is Active", &obj.isActive);
            
            
            ImGui::Separator();
            ImGui::Text("Transform");
            
            
            float pos[3] = { obj.position.x, obj.position.y, obj.position.z };
            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                obj.position = glm::vec3(pos[0], pos[1], pos[2]);
            }
            
            
            glm::vec3 euler = glm::degrees(glm::eulerAngles(obj.rotation));
            float rot[3] = { euler.x, euler.y, euler.z };
            if (ImGui::DragFloat3("Rotation", rot, 1.0f)) {
                 obj.rotation = glm::quat(glm::radians(glm::vec3(rot[0], rot[1], rot[2])));
            }
            
            
            float scale[3] = { obj.scale.x, obj.scale.y, obj.scale.z };
            if (ImGui::DragFloat3("Scale", scale, 0.1f)) {
                obj.scale = glm::vec3(scale[0], scale[1], scale[2]);
            }
            
            if (obj.meshType != MeshType::None) {
                ImGui::Separator();
                ImGui::Text("Material");
                ImGui::ColorEdit3("Albedo", &obj.material.albedo[0]);
                ImGui::SliderFloat("Metallic", &obj.material.metallic, 0.0f, 1.0f);
                ImGui::SliderFloat("Roughness", &obj.material.roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("AO", &obj.material.ao, 0.0f, 1.0f);
                ImGui::DragFloat("Shininess", &obj.material.shininess, 1.0f, 1.0f, 256.0f);
                ImGui::Checkbox("Use Texture", &obj.material.useTexture);

                
                ImGui::Text("Diffuse Texture:");
                std::string diffLabel = obj.material.diffuseTexture.empty() 
                    ? "[ Drop texture here ]##diffusedrop" 
                    : std::filesystem::path(obj.material.diffuseTexture).filename().string() + "##diffusedrop";
                ImGui::Button(diffLabel.c_str(), ImVec2(-40, 0));
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_FILE")) {
                        std::string texPath((const char*)payload->Data, payload->DataSize - 1);
                        obj.material.diffuseTexture = texPath;
                        obj.material.useTexture = true;
                        
                        obj.material.albedo = glm::vec3(1.0f);
                        
                        obj.mesh.textures.clear();
                        obj.mesh.textures.push_back(Texture(texPath.c_str(), "diffuse"));
                        Logger::AddLog("Set diffuse texture: %s", texPath.c_str());
                    }
                    ImGui::EndDragDropTarget();
                }
                if (!obj.material.diffuseTexture.empty()) {
                    ImGui::SameLine();
                    if (ImGui::Button("X##ClearDiff")) {
                        obj.material.diffuseTexture = "";
                        obj.mesh.textures.clear();
                        Logger::AddLog("Cleared diffuse texture");
                    }
                }

                
                ImGui::Text("Specular Texture:");
                std::string specLabel = obj.material.specularTexture.empty() 
                    ? "[ Drop texture here ]##speculardrop" 
                    : std::filesystem::path(obj.material.specularTexture).filename().string() + "##speculardrop";
                ImGui::Button(specLabel.c_str(), ImVec2(-40, 0));
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_FILE")) {
                        std::string texPath((const char*)payload->Data, payload->DataSize - 1);
                        obj.material.specularTexture = texPath;
                        obj.mesh.textures.push_back(Texture(texPath.c_str(), "specular"));
                        Logger::AddLog("Set specular texture: %s", texPath.c_str());
                    }
                    ImGui::EndDragDropTarget();
                }
                if (!obj.material.specularTexture.empty()) {
                    ImGui::SameLine();
                    if (ImGui::Button("X##ClearSpec")) {
                        obj.material.specularTexture = "";
                        
                        obj.mesh.textures.erase(
                            std::remove_if(obj.mesh.textures.begin(), obj.mesh.textures.end(),
                                [](const Texture& t) { return std::string(t.type) == "specular"; }),
                            obj.mesh.textures.end());
                        Logger::AddLog("Cleared specular texture");
                    }
                }
                ImGui::Separator();
                ImGui::Text("Custom Render Shader:");
                char shaderNameBuffer[128] = "";
                strncpy(shaderNameBuffer, obj.material.customShaderName.c_str(), sizeof(shaderNameBuffer) - 1);
                if (ImGui::InputText("##CustomShaderName", shaderNameBuffer, sizeof(shaderNameBuffer))) {
                    obj.material.customShaderName = shaderNameBuffer;
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHADER_FILE")) {
                        std::string shaderPath((const char*)payload->Data, payload->DataSize - 1);
                        std::filesystem::path p(shaderPath);
                        obj.material.customShaderName = p.stem().string();
                        Logger::AddLog("Assigned custom shader: %s", obj.material.customShaderName.c_str());
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine();
                if (ImGui::Button("X##ClearShader")) {
                    obj.material.customShaderName = "";
                }
            }
            
            if (obj.meshType != MeshType::None) {
                ImGui::Separator();
                ImGui::Text("Physics");
                ImGui::Checkbox("Enable Gravity", &obj.useGravity);
                ImGui::Checkbox("Is Static", &obj.isStatic);
                ImGui::Checkbox("Enable Collision", &obj.enableCollision);
                
                float com[3] = { obj.centerOfMassOffset.x, obj.centerOfMassOffset.y, obj.centerOfMassOffset.z };
                if (ImGui::DragFloat3("COM Offset", com, 0.01f)) {
                    obj.centerOfMassOffset = glm::vec3(com[0], com[1], com[2]);
                }
                
                float vel[3] = { obj.velocity.x, obj.velocity.y, obj.velocity.z };
                ImGui::BeginDisabled();
                ImGui::DragFloat3("Velocity", vel, 0.0f);
                float aVel[3] = { obj.angularVelocity.x, obj.angularVelocity.y, obj.angularVelocity.z };
                ImGui::DragFloat3("Angular Vel", aVel, 0.0f);
                ImGui::EndDisabled();

                ImGui::DragFloat("Mass", &obj.mass, 0.1f, 0.01f, 100.0f);
                ImGui::DragFloat("Friction", &obj.friction, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Bounciness", &obj.restitution, 0.01f, 0.0f, 1.0f);
            }
            
            if (obj.meshType != MeshType::None) {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.6f, 0.9f, 0.6f, 1.0f), "Acoustic Surface");
                ImGui::SliderFloat("Hardness", &obj.acousticMaterial.hardness, 0.0f, 1.0f, "%.2f");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("0 = soft (carpet, foam)\n1 = hard (concrete, metal)\nAffects echo and reflections");
                ImGui::SliderFloat("Absorption", &obj.acousticMaterial.absorption, 0.0f, 1.0f, "%.2f");
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("How much sound energy is absorbed\n0 = none, 1 = fully absorbed");
                ImGui::Checkbox("Acoustic Obstacle", &obj.acousticMaterial.isAcousticObstacle);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Can this object block/reflect sound?");
            }
            
            
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Audio");
            ImGui::Checkbox("Enable Audio", &obj.hasAudio);
            if (obj.hasAudio) {
                ImGui::Indent();
                
                bool isVideoOverrides = (obj.hasScreen && obj.screen.type == ScreenType::Video);
                if (!isVideoOverrides) {
                    ImGui::Text("Audio File:");
                    std::string audioLabel = obj.audio.filePath.empty() 
                        ? "[ Drop audio here ]##audiodrop" 
                        : std::filesystem::path(obj.audio.filePath).filename().string() + "##audiodrop";
                    
                    ImGui::Button(audioLabel.c_str(), ImVec2(-40, 0));
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AUDIO_FILE")) {
                            std::string audioPath((const char*)payload->Data, payload->DataSize - 1);
                            obj.audio.filePath = audioPath;
                            Logger::AddLog("Assigned audio file: %s", audioPath.c_str());
                        }
                        ImGui::EndDragDropTarget();
                    }
                    if (!obj.audio.filePath.empty()) {
                        ImGui::SameLine();
                        if (ImGui::Button("X##ClearAudio")) {
                            obj.audio.filePath = "";
                        }
                    }

                    ImGui::SliderFloat("Volume", &obj.audio.volume, 0.0f, 1.0f);
                    ImGui::SliderFloat("Pitch", &obj.audio.pitch, 0.1f, 2.0f);
                    ImGui::Checkbox("Looping", &obj.audio.looping);
                    ImGui::Checkbox("Play On Awake", &obj.audio.playOnAwake);
                } else {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[ Base audio controlled by Video Component ]");
                }

                const char* audioTypes[] = { "Directional", "Ambience" };
                int currentAudioType = (int)obj.audio.type;
                if (ImGui::Combo("Type", &currentAudioType, audioTypes, IM_ARRAYSIZE(audioTypes))) {
                    obj.audio.type = (AudioType)currentAudioType;
                }

                if (obj.audio.type == AudioType::Directional) {
                    ImGui::DragFloat("Min Distance", &obj.audio.minDistance, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("Max Distance", &obj.audio.maxDistance, 1.0f, 1.0f, 1000.0f);
                    
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Physics Audio");
                    
                    
                    ImGui::Checkbox("Doppler Effect", &obj.audio.enableDoppler);
                    if (obj.audio.enableDoppler) {
                        ImGui::SliderFloat("Doppler Factor", &obj.audio.dopplerFactor, 0.0f, 5.0f, "%.1f");
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Higher = more extreme pitch shift\nfrom object movement");
                    }
                    
                    
                    ImGui::Checkbox("Reverb / Echo", &obj.audio.enableReverb);
                    if (obj.audio.enableReverb && obj.audio.playing) {
                        ImGui::BeginDisabled();
                        ImGui::SliderFloat("Reverb Mix##ro", &obj.audio.reverbMix, 0.0f, 1.0f);
                        ImGui::SliderFloat("Echo Delay##ro", &obj.audio.echoDelay, 0.0f, 0.5f, "%.3f s");
                        ImGui::SliderFloat("Echo Decay##ro", &obj.audio.echoDecay, 0.0f, 1.0f);
                        ImGui::EndDisabled();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Computed from nearby surfaces.\nHarder surfaces = more echo.");
                    
                    
                    ImGui::Checkbox("Occlusion", &obj.audio.enableOcclusion);
                    if (obj.audio.enableOcclusion && obj.audio.playing) {
                        ImGui::BeginDisabled();
                        ImGui::SliderFloat("Occlusion##ro", &obj.audio.occlusionFactor, 0.0f, 1.0f);
                        ImGui::EndDisabled();
                    }
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sound blocked by objects between\nsource and listener.");
                }

                ImGui::Spacing();
                if (obj.audio.playing) {
                    if (ImGui::Button("Stop Preview")) {
                        AudioEngine::StopObjectAudio(obj);
                    }
                } else {
                    if (ImGui::Button("Play Preview")) {
                        AudioEngine::PlayObjectAudio(obj);
                    }
                }

                ImGui::Unindent();
            }

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Scripts");
            
            
            int numScripts = std::min((int)obj.behaviors.size(), (int)obj.scriptNames.size());
            for (int j = 0; j < numScripts; j++) {
                ImGui::PushID(j);
                ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "  ● %s", obj.scriptNames[j].c_str());
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                if (ImGui::SmallButton("Remove")) {
                    obj.behaviors.erase(obj.behaviors.begin() + j);
                    obj.scriptNames.erase(obj.scriptNames.begin() + j);
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                    break; 
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
            }
            
            while (obj.scriptNames.size() > obj.behaviors.size()) obj.scriptNames.pop_back();
            while (obj.behaviors.size() > obj.scriptNames.size()) obj.behaviors.pop_back();

            
            ImGui::Spacing();
            ImGui::Text("Add Script:");
            auto availableScripts = BehaviorRegistry::GetAvailableScripts();
            
            std::vector<std::string> allScriptOptions;
            allScriptOptions.push_back("(None)");
            for (auto& s : availableScripts) allScriptOptions.push_back(s);

            
            namespace fs = std::filesystem;
            std::string projectRoot = Application::Get().GetProjectRoot();
            if (!projectRoot.empty() && fs::exists(projectRoot)) {
                try {
                    for (auto& entry : fs::recursive_directory_iterator(projectRoot)) {
                        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                            
                            std::string relPath = fs::relative(entry.path(), projectRoot).string();
                            if (relPath.find("Build") == 0) continue;
                            
                            std::string stem = entry.path().stem().string();
                            
                            bool alreadyListed = false;
                            for (auto& existing : allScriptOptions) {
                                if (existing == stem) { alreadyListed = true; break; }
                            }
                            if (!alreadyListed) {
                                allScriptOptions.push_back(stem);
                            }
                        }
                    }
                } catch (...) {}
            }

            {
                static int addScriptIdx = 0;
                if (addScriptIdx >= (int)allScriptOptions.size()) addScriptIdx = 0;
                
                std::vector<const char*> items;
                for (auto& s : allScriptOptions) items.push_back(s.c_str());
                
                static bool isCompiling = false;
                static std::string compileScriptName = "";
                static std::string compileScriptPath = "";
                static GameObject* compileTargetObj = nullptr;
                static int compileScriptFramesWait = 0;

                ImGui::Combo("##AddScript", &addScriptIdx, items.data(), (int)items.size());
                ImGui::SameLine();
                
                if (isCompiling) {
                    ImGui::Button("Compiling...");
                    if (compileScriptFramesWait > 1) { 
                        fs::path engineRoot = fs::path("/home/light/Documents/C3D/Calcium3D");
                        if (ScriptCompiler::CompileAndLoad(compileScriptPath, engineRoot.string())) {
                            auto behavior = BehaviorRegistry::Create(compileScriptName);
                            if (behavior && compileTargetObj) {
                                behavior->gameObject = compileTargetObj;
                                compileTargetObj->behaviors.push_back(std::move(behavior));
                                compileTargetObj->scriptNames.push_back(compileScriptName);
                                Logger::AddLog("Attached '%s' to '%s'", compileScriptName.c_str(), compileTargetObj->name.c_str());
                            }
                        } else {
                            Logger::AddLog("[ERROR] Failed to compile or attach '%s'", compileScriptName.c_str());
                        }
                        isCompiling = false;
                        compileTargetObj = nullptr;
                    } else {
                        compileScriptFramesWait++;
                    }
                } else {
                    if (ImGui::Button("Attach")) {
                    if (addScriptIdx == 0) { 
                        obj.behaviors.clear();
                        obj.scriptNames.clear();
                        Logger::AddLog("Removed all scripts from '%s'", obj.name.c_str());
                    } else {
                        std::string sName = allScriptOptions[addScriptIdx];
                        auto behavior = BehaviorRegistry::Create(sName);
                        
                        if (!behavior) {
                            
                            std::string projectRoot = Application::Get().GetProjectRoot();
                            if (!projectRoot.empty()) {
                                for (auto& entry : fs::recursive_directory_iterator(projectRoot)) {
                                    if (entry.is_regular_file() && entry.path().stem().string() == sName && entry.path().extension() == ".cpp") {
                                        std::string relPath = fs::relative(entry.path(), projectRoot).string();
                                        if (relPath.find("Build") == 0) continue;
                                        
                                        isCompiling = true;
                                        compileScriptName = sName;
                                        compileScriptPath = entry.path().string();
                                        compileTargetObj = &obj;
                                        compileScriptFramesWait = 0;
                                        Logger::AddLog("Compiling %s... please wait", sName.c_str());
                                        break;
                                    }
                                }
                            }
                        } else {
                            
                            behavior->gameObject = &obj;
                            obj.behaviors.push_back(std::move(behavior));
                            obj.scriptNames.push_back(sName);
                            Logger::AddLog("Attached '%s' to '%s'", sName.c_str(), obj.name.c_str());
                        }
                    }
                    }
                }
            }

            
            ImGui::Separator();
            ImGui::Checkbox("Has Camera", &obj.hasCamera);
            if (obj.hasCamera) {
                if (ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Enabled##Cam", &obj.camera.enabled);
                    ImGui::DragFloat("FOV", &obj.camera.fov, 1.0f, 10.0f, 120.0f);
                    ImGui::DragFloat("Near", &obj.camera.nearPlane, 0.01f, 0.01f, 1.0f);
                    ImGui::DragFloat("Far", &obj.camera.farPlane, 1.0f, 1.0f, 5000.0f);
                    
                    int res[2] = { obj.camera.resolutionX, obj.camera.resolutionY };
                    if (ImGui::DragInt2("Resolution", res, 1, 64, 4096)) {
                        obj.camera.resolutionX = res[0];
                        obj.camera.resolutionY = res[1];
                    }
                    
                    if (obj.camera.renderTexture != 0) {
                        ImGui::Text("Render Texture ID: %d", obj.camera.renderTexture);
                        ImGui::Image((void*)(intptr_t)obj.camera.renderTexture, ImVec2(200, 200), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
                    }
                }
            }
            
            ImGui::Separator();
            ImGui::Checkbox("Has Screen", &obj.hasScreen);
            if (obj.hasScreen) {
                if (ImGui::CollapsingHeader("Screen Component", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("Enabled##Screen", &obj.screen.enabled);
                    ImGui::DragFloat("Brightness", &obj.screen.brightness, 0.1f, 0.0f, 100.0f);

                    const char* typeOptions[] = { "None", "Image", "Video", "CameraFeed" };
                    int typeIdx = static_cast<int>(obj.screen.type);
                    if (ImGui::Combo("Source Type", &typeIdx, typeOptions, 4)) {
                        obj.screen.type = static_cast<ScreenType>(typeIdx);
                    }
                    
                    ImGui::Spacing();
                    
                    if (obj.screen.type == ScreenType::Image || obj.screen.type == ScreenType::Video) {
                        char buf[512];
                        strcpy(buf, obj.screen.filePath.c_str());
                        if (ImGui::InputText((obj.screen.type == ScreenType::Video ? "Video Path" : "Image Path"), buf, sizeof(buf))) {
                            obj.screen.filePath = buf;
                        }
                    } else if (obj.screen.type == ScreenType::CameraFeed) {
                        auto& allObjects = scene.GetObjects();
                        std::string currentCamName = "None";
                        if (obj.screen.targetCameraIndex >= 0 && obj.screen.targetCameraIndex < (int)allObjects.size()) {
                            currentCamName = allObjects[obj.screen.targetCameraIndex].name;
                        }
                        ImGui::Text("Target Camera: %s", currentCamName.c_str());
                    }

                    ImGui::Button("[ Drop File or Camera from Hierarchy Here ]", ImVec2(-1, 0));
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                            const char* path = (const char*)payload->Data;
                            obj.screen.filePath = path;
                            std::string ext = std::filesystem::path(path).extension().string();
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            if (ext == ".mp4" || ext == ".mkv" || ext == ".avi" || ext == ".mov") 
                                obj.screen.type = ScreenType::Video;
                            else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga")
                                obj.screen.type = ScreenType::Image;
                            Logger::AddLog("Screen: Assigned file %s", path);
                        }
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_OBJ")) {
                            int camIdx = *(int*)payload->Data;
                            auto& allObjects = scene.GetObjects();
                            if (camIdx >= 0 && camIdx < (int)allObjects.size() && allObjects[camIdx].hasCamera) {
                                obj.screen.targetCameraIndex = camIdx;
                                obj.screen.type = ScreenType::CameraFeed;
                                Logger::AddLog("Screen: Assigned Camera Feed from %s", allObjects[camIdx].name.c_str());
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }

                    if (obj.screen.type == ScreenType::Video) {
                        if (ImGui::TreeNode("Playback Settings")) {
                            ImGui::Checkbox("Loop##Vid", &obj.screen.videoLoop);
                            ImGui::SameLine();
                            ImGui::Checkbox("Start Paused", &obj.screen.videoPaused);
                            ImGui::Checkbox("Retain Aspect Ratio", &obj.screen.videoKeepAspect);
                            
                            ImGui::DragFloat("Speed", &obj.screen.videoPlaybackSpeed, 0.05f, 0.1f, 10.0f);
                            ImGui::SliderFloat("Volume##Video", &obj.screen.videoVolume, 0.0f, 1.0f);
                            ImGui::TreePop();
                        }
                    } else if (obj.screen.type == ScreenType::CameraFeed) {
                        auto& allObjects = scene.GetObjects();
                        std::vector<const char*> camNames;
                        std::vector<int> camIndices;
                        for(int i=0; i<(int)allObjects.size(); i++) {
                            if(allObjects[i].hasCamera) {
                                camNames.push_back(allObjects[i].name.c_str());
                                camIndices.push_back(i);
                            }
                        }
                        if(!camNames.empty()) {
                            int currentCamIdx = -1;
                            for(int i=0; i<(int)camIndices.size(); i++) {
                                if(camIndices[i] == obj.screen.targetCameraIndex) { currentCamIdx = i; break; }
                            }
                            if (ImGui::Combo("Select Camera", &currentCamIdx, camNames.data(), camNames.size())) {
                                obj.screen.targetCameraIndex = camIndices[currentCamIdx];
                            }
                        } else {
                            ImGui::TextDisabled("No cameras in scene.");
                        }
                    }
                }
            }

            ImGui::Separator();
            ImGui::Text("Actions");
            static glm::vec3 impulseForce(0.0f, 10.0f, 0.0f);
            ImGui::DragFloat3("Impulse Vector", &impulseForce[0], 0.1f);
            if (ImGui::Button("Apply Impulse (Kick)")) {
                obj.ApplyImpulse(impulseForce);
                Logger::AddLog("Applied impulse to %s", obj.name.c_str());
            }
            
            ImGui::Separator();
            if (ImGui::Button("Delete Object")) {
                scene.RemoveObject(selectedCube);
                selectedCube = -1;
            }
        }
    } else if (selectedPointLightIndex >= 0) {
        auto& pl = scene.GetPointLights();
        if (selectedPointLightIndex < (int)pl.size()) {
            auto& light = pl[selectedPointLightIndex];
            ImGui::Text("Type: Point Light %d", selectedPointLightIndex);
            
            ImGui::Checkbox("Enabled", &light.enabled);
            
            float pos[3] = { light.position.x, light.position.y, light.position.z };
            if (ImGui::DragFloat3("Position", pos, 0.1f)) light.position = glm::vec3(pos[0], pos[1], pos[2]);
            
            float color[4] = { light.color.r, light.color.g, light.color.b, light.color.a };
            if (ImGui::ColorEdit4("Color", color)) light.color = glm::vec4(color[0], color[1], color[2], color[3]);
            
            ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 20.0f);
            
            ImGui::Separator();
            ImGui::Text("Attenuation");
            ImGui::DragFloat("Constant", &light.constant, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Linear", &light.linear, 0.001f, 0.0f, 1.0f);
            ImGui::DragFloat("Quadratic", &light.quadratic, 0.0001f, 0.0f, 1.0f);
            
            ImGui::Separator();
            if (ImGui::Button("Delete Light")) {
                scene.RemovePointLight(selectedPointLightIndex);
                selectedPointLightIndex = -1; 
                Logger::AddLog("Deleted Point Light");
            }
        }
    } else if (isLightSelected) {
        ImGui::Text("Type: Legacy Light Source (Unused)");
        float pos[3] = { lightPos.x, lightPos.y, lightPos.z };
        if (ImGui::DragFloat3("Position", pos, 0.1f)) lightPos = glm::vec3(pos[0], pos[1], pos[2]);
    } else {
        ImGui::Text("No object selected.");
        
        
        namespace fs = std::filesystem;
        if (!m_SelectedContentFile.empty() && fs::exists(m_SelectedContentFile)) {
            std::string ext = fs::path(m_SelectedContentFile).extension().string();
            if (ext == ".cpp") {
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.3f, 0.5f, 0.9f, 1.0f), "Script: %s", 
                    fs::path(m_SelectedContentFile).filename().c_str());
                ImGui::Text("Path: %s", m_SelectedContentFile.c_str());
                
                ImGui::Separator();
                ImGui::Text("Attach to Object:");
                
                auto& objects = scene.GetObjects();
                if (!objects.empty()) {
                    static int attachTarget = 0;
                    std::vector<const char*> objNames;
                    for (auto& obj : objects) objNames.push_back(obj.name.c_str());
                    
                    ImGui::Combo("##AttachTarget", &attachTarget, objNames.data(), (int)objNames.size());
                    
                    
                    auto availableScripts = BehaviorRegistry::GetAvailableScripts();
                    ImGui::Text("Quick Attach:");
                    for (auto& sName : availableScripts) {
                        std::string btnLabel = "Attach " + sName + " →";
                        if (ImGui::Button(btnLabel.c_str())) {
                            if (attachTarget >= 0 && attachTarget < (int)objects.size()) {
                                auto& obj = objects[attachTarget];
                                auto behavior = BehaviorRegistry::Create(sName);
                                if (behavior) {
                                    behavior->gameObject = &obj;
                                    obj.behaviors.push_back(std::move(behavior));
                                    obj.scriptNames.push_back(sName);
                                    Logger::AddLog("Attached '%s' to '%s'", sName.c_str(), obj.name.c_str());
                                }
                            }
                        }
                    }
                } else {
                    ImGui::TextDisabled("No objects in scene. Add an object first.");
                }
            }
        }

        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Quick Script Attach");
        ImGui::Text("Select an object and script to attach:");
        
        auto& objects = scene.GetObjects();
        if (!objects.empty()) {
            static int quickObj = 0;
            std::vector<const char*> objNames;
            for (auto& obj : objects) objNames.push_back(obj.name.c_str());
            ImGui::Combo("Object##QuickAttach", &quickObj, objNames.data(), (int)objNames.size());

            auto availableScripts = BehaviorRegistry::GetAvailableScripts();
            if (!availableScripts.empty()) {
                static int quickScript = 0;
                std::vector<const char*> scriptItems;
                for (auto& s : availableScripts) scriptItems.push_back(s.c_str());
                ImGui::Combo("Script##QuickAttach", &quickScript, scriptItems.data(), (int)scriptItems.size());

                if (ImGui::Button("Attach Script to Object")) {
                    if (quickObj >= 0 && quickObj < (int)objects.size()) {
                        auto& obj = objects[quickObj];
                        std::string sName = availableScripts[quickScript];
                        auto behavior = BehaviorRegistry::Create(sName);
                        if (behavior) {
                            behavior->gameObject = &obj;
                            obj.behaviors.push_back(std::move(behavior));
                            obj.scriptNames.push_back(sName);
                            Logger::AddLog("Attached '%s' to '%s'", sName.c_str(), obj.name.c_str());
                        }
                    }
                }
            }
        }
    }
    
    ImGui::End();
}

void EditorLayer::DrawSettings(Camera& camera) {
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Settings", nullptr, lockFlags);
    
    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::DragFloat("Speed", &camera.speed, 0.1f, 0.1f, 10.0f);
        ImGui::DragFloat("Sensitivity", &camera.sensitivity, 1.0f, 10.f, 200.0f);
        ImGui::DragFloat("FOV", &camera.FOV, 1.0f, 30.0f, 120.0f);
    }
    if (ImGui::CollapsingHeader("Physics")) {
        ImGui::Checkbox("Global Physics Enabled", &PhysicsEngine::GlobalPhysicsEnabled);
        ImGui::Checkbox("Global Gravity Enabled", &PhysicsEngine::GlobalGravityEnabled);
        ImGui::Checkbox("Use Impulses", &PhysicsEngine::ImpulseEnabled);
        ImGui::Checkbox("Use Center of Mass", &PhysicsEngine::GlobalCOMEnabled);
        ImGui::DragFloat("Air Resistance", &PhysicsEngine::GlobalAirResistance, 0.01f, 0.0f, 10.0f);
        ImGui::SliderInt("Sub-Steps (High Speed Precision)", &PhysicsEngine::SubSteps, 1, 10);
        ImGui::DragFloat("Linear Damping", &PhysicsEngine::LinearDamping, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Angular Damping", &PhysicsEngine::AngularDamping, 0.001f, 0.0f, 1.0f);
        ImGui::Checkbox("Show Hitboxes", &HitboxGraphics::ShowHitboxes);
        
        float grav[3] = { PhysicsEngine::Gravity.x, PhysicsEngine::Gravity.y, PhysicsEngine::Gravity.z };
        if (ImGui::DragFloat3("Gravity Vector", grav, 0.1f)) {
            PhysicsEngine::Gravity = glm::vec3(grav[0], grav[1], grav[2]);
        }
        
        float gAcc[3] = { PhysicsEngine::GlobalAcceleration.x, PhysicsEngine::GlobalAcceleration.y, PhysicsEngine::GlobalAcceleration.z };
        if (ImGui::DragFloat3("Global Acceleration", gAcc, 0.1f)) {
            PhysicsEngine::GlobalAcceleration = glm::vec3(gAcc[0], gAcc[1], gAcc[2]);
        }
    }
    
    if (ImGui::CollapsingHeader("Rendering")) {
        ImGui::SliderFloat("Render Distance", &camera.farPlane, 10.0f, 1000.0f, "%.1f");
        ImGui::SliderFloat("Near Plane", &camera.nearPlane, 0.01f, 1.0f, "%.3f");
        ImGui::Checkbox("Wireframe", &wireframe);
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        if (ImGui::Checkbox("VSync", &vsync)) {
            glfwSwapInterval(vsync ? 1 : 0);
        }
        
        
        const char* msaaOptions[] = { "Off", "2x", "4x", "8x" };
        
        int currentMsaaIndex = 0;
        if (msaaSamples == 2) currentMsaaIndex = 1;
        else if (msaaSamples == 4) currentMsaaIndex = 2;
        else if (msaaSamples == 8) currentMsaaIndex = 3;
        
        if (ImGui::Combo("MSAA", &currentMsaaIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions))) {
            if (currentMsaaIndex == 0) msaaSamples = 0;
            else if (currentMsaaIndex == 1) msaaSamples = 2;
            else if (currentMsaaIndex == 2) msaaSamples = 4;
            else if (currentMsaaIndex == 3) msaaSamples = 8;
        }

        if (msaaSamples > 0) {
            ImGui::Indent();
            ImGui::Text("Apply MSAA To:");
            ImGui::Checkbox("Sky Pass", &msaaSkyPass);
            ImGui::Checkbox("Geometry Pass", &msaaGeometryPass);
            ImGui::Checkbox("Transparency Pass", &msaaTransparencyPass);
            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::LabelText("", "Environment Mode");
        const char* skyModes[] = {"Cubemap", "Dynamic Sky"};
        static int skyMode = 0;
        if (showGradientSky) skyMode = 1; else skyMode = 0;
        
        if (ImGui::Combo("Sky Mode", &skyMode, skyModes, IM_ARRAYSIZE(skyModes))) {
            showSkybox = (skyMode == 0);
            showGradientSky = (skyMode == 1);
        }
        
        ImGui::Checkbox("Use Gradient Sky", &showGradientSky);
        
        ImGui::Checkbox("Water", &showWater);
        ImGui::Checkbox("Clouds", &showClouds);
    }
    
    if (ImGui::CollapsingHeader("Environment Settings")) {
        ImGui::Text("Time Settings");
        ImGui::DragFloat("Time Speed", &timeSpeed, 0.1f, 0.0f, 20.0f);
        ImGui::Checkbox("Pause Time", &isTimePaused);
        
        int h = (int)timeOfDay;
        float m_f = (timeOfDay - h) * 60.0f;
        int m = (int)m_f;
        int s = (int)((m_f - m) * 60.0f);
        
        ImGui::Text("Time: %02d:%02d:%02d", h, m, s);
        
        bool timeChanged = false;
        if (ImGui::SliderInt("Hour", &h, 0, 23)) timeChanged = true;
        if (ImGui::SliderInt("Minute", &m, 0, 59)) timeChanged = true;
        
        if (timeChanged) {
            timeOfDay = h + m / 60.0f + s / 3600.0f;
        }
        ImGui::Separator();

        ImGui::Text("Sun Settings");
        ImGui::Checkbox("Sun Enabled", &sunEnabled);
        
        float sunColorArray[4] = { sunColor.r, sunColor.g, sunColor.b, sunColor.a };
        if (ImGui::ColorEdit4("Sun Color", sunColorArray)) {
            sunColor = glm::vec4(sunColorArray[0], sunColorArray[1], sunColorArray[2], sunColorArray[3]);
        }
        
        ImGui::SliderFloat("Sun Intensity", &sunIntensity, 0.0f, 5.0f);
        
        ImGui::Separator();
        ImGui::Text("Moon Settings");
        ImGui::Checkbox("Moon Enabled", &moonEnabled);
        
        float moonColorArray[4] = { moonColor.r, moonColor.g, moonColor.b, moonColor.a };
        if (ImGui::ColorEdit4("Moon Color", moonColorArray)) {
            moonColor = glm::vec4(moonColorArray[0], moonColorArray[1], moonColorArray[2], moonColorArray[3]);
        }
        
        ImGui::SliderFloat("Moon Intensity", &moonIntensity, 0.0f, 5.0f);

        ImGui::Separator();
        ImGui::Text("Bloom Settings");
        ImGui::SliderFloat("Sun Bloom", &sunBloom, 0.0f, 0.1f);
        ImGui::SliderFloat("Moon Bloom", &moonBloom, 0.0f, 0.1f);
        
        ImGui::Separator();
        ImGui::Text("Global Texture Settings");
        ImGui::SliderFloat("Texture Tiling", &globalTilingFactor, 0.1f, 10.0f);
    }
    
    if (showClouds && ImGui::CollapsingHeader("Cloud Settings")) {
        const char* cloudModes[] = {"2D Clouds", "Volumetric Clouds"};
        ImGui::Combo("Cloud Mode", &cloudMode, cloudModes, IM_ARRAYSIZE(cloudModes));
        ImGui::SliderFloat("Cloud Height", &cloud2dHeight, 5.0f, 90.0f);

        if (cloudMode == 0) { 
            ImGui::ColorEdit3("Cloud Color", glm::value_ptr(cloudColor));
            ImGui::SliderFloat("Cloud Cover", &cloudCover, 0.0f, 2.0f);
            ImGui::SliderFloat("Cloud Speed", &cloudSpeed, 0.0f, 5.0f);
            ImGui::SliderFloat("Tiling", &cloudTiling, 0.1f, 10.0f);
            ImGui::SliderFloat("Density", &cloudDensity, 0.0f, 1.0f);
            ImGui::SliderFloat("Size", &cloudSize, 0.1f, 10.0f);
            ImGui::SliderFloat("Randomness", &cloudRandomness, 0.0f, 1.0f);
        } else { 
            ImGui::SliderFloat("Density", &volCloudDensity, 0.0f, 2.0f);
            ImGui::SliderFloat("Step Size", &volCloudStepSize, 0.01f, 2.0f);
            ImGui::SliderFloat("Cover", &volCloudCover, 0.0f, 1.0f);
            ImGui::SliderFloat("Speed", &volCloudSpeed, 0.0f, 0.2f);
            ImGui::SliderFloat("Detail", &volCloudDetail, 1.0f, 10.0f);
            const char* qualityModes[] = {"Performance", "Quality"};
            ImGui::Combo("Quality", &volCloudQuality, qualityModes, IM_ARRAYSIZE(qualityModes));
        }
    }
    
    if (showWater && ImGui::CollapsingHeader("Water Settings")) {
        ImGui::SliderFloat("Height", &waterHeight, -10.0f, 10.0f);
        ImGui::SliderFloat("Wave Speed", &waveSpeed, 0.0f, 5.0f);
        ImGui::SliderFloat("Wave Strength", &waveStrength, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &waterShininess, 1.0f, 256.0f);
        ImGui::ColorEdit3("Water Color", glm::value_ptr(waterColor));
        const char* waveSystems[] = { "Blinn-Wyvill", "Gerstner" };
        ImGui::Combo("Wave System", &waveSystem, waveSystems, IM_ARRAYSIZE(waveSystems));
    }
    
    if (ImGui::CollapsingHeader("Engine Settings")) {
        ImGui::Text("UI Theme");
        const char* themes[] = { "Dark", "Light" };
        int currentTheme = darkTheme ? 0 : 1;
        if (ImGui::Combo("Theme", &currentTheme, themes, IM_ARRAYSIZE(themes))) {
            darkTheme = (currentTheme == 0);
            if (darkTheme) {
                ApplyProfessionalDarkTheme();
            } else {
                ApplyProfessionalLightTheme();
            }
        }
        
        bool mtEnabled = ThreadManager::IsEnabled();
        if (ImGui::Checkbox("Multithreading", &mtEnabled)) {
            ThreadManager::SetEnabled(mtEnabled);
        }
        
        ImGui::Separator();
        ImGui::Text("Layout Mode");
        const char* layoutModes[] = { "Fixed", "Resizable" };
        int currentMode = fixedLayout ? 0 : 1;
        if (ImGui::Combo("UI Mode", &currentMode, layoutModes, IM_ARRAYSIZE(layoutModes))) {
            fixedLayout = (currentMode == 0);
        }
        if (fixedLayout) {
            ImGui::TextDisabled("Panels are locked in place.");
        } else {
            ImGui::TextDisabled("Drag panels to rearrange.");
        }



        std::string activeScreenName = GameStateManager::GetStateName(m_PreviewState);
        
        ImGui::Separator();
        ImGui::Text("Info");
        ImGui::Text("Calcium3D Engine v0.1");
        ImGui::Text("OpenGL %s", glGetString(GL_VERSION));
        ImGui::Text("GPU: %s", glGetString(GL_RENDERER));
    }
    
    if (ImGui::CollapsingHeader("UI Editor")) {
        DrawUIEditor();
    }
    

    if (ImGui::CollapsingHeader("State Manager")) {
    
    ImGui::Text("Load Hierarchy (Start State)");
    auto& allStates = GameStateManager::GetAllStates();
    std::vector<const char*> stateNames;
    std::vector<int> stateIds;
    for (auto const& [id, name] : allStates) {
        stateNames.push_back(name.c_str());
        stateIds.push_back(id);
    }

    int currentStart = (int)Application::Get().m_StartGameState;
    int startIndex = 0;
    for(int i=0; i < stateIds.size(); i++) if(stateIds[i] == currentStart) startIndex = i;

    if (ImGui::Combo("Start State", &startIndex, stateNames.data(), (int)stateNames.size())) {
        Application::Get().m_StartGameState = (GameState)stateIds[startIndex];
    }

    ImGui::Separator();
    ImGui::Text("Editor Preview State (Live Filter)");
    
    int previewIndex = 0;
    for(int i=0; i < stateIds.size(); i++) if(stateIds[i] == m_PreviewState) previewIndex = i;

    if (ImGui::Combo("Preview State", &previewIndex, stateNames.data(), (int)stateNames.size())) {
        m_PreviewState = stateIds[previewIndex];
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filters the viewport UI to only show elements for this state.");
    
    if (ImGui::Button("Save Camera & Objects to State")) {
        std::string stateName = GameStateManager::GetStateName(m_PreviewState);
        nlohmann::json statesJson = nlohmann::json::object();
        std::string statesFile = (std::filesystem::path(m_ProjectRoot) / "states.json").string();
        
        if (std::filesystem::exists(statesFile)) {
            std::ifstream i(statesFile);
            if (i.is_open()) {
                auto parsed = nlohmann::json::parse(i, nullptr, false);
                if (!parsed.is_discarded() && parsed.is_object()) {
                    statesJson = parsed;
                }
                i.close();
            }
        }

        nlohmann::json stateData;
        stateData["camera"]["position"] = {camera.Position.x, camera.Position.y, camera.Position.z};
        stateData["camera"]["orientation"] = {camera.Orientation.x, camera.Orientation.y, camera.Orientation.z};
        stateData["camera"]["yaw"] = camera.yaw;
        stateData["camera"]["pitch"] = camera.pitch;
        stateData["camera"]["fov"] = camera.FOV;

        nlohmann::json activeObjects = nlohmann::json::array();
        auto& objects = Application::Get().GetScene()->GetObjects();
        for (auto& obj : objects) {
            if (obj.isActive) activeObjects.push_back(obj.name);
        }
        stateData["activeObjects"] = activeObjects;

        statesJson[stateName] = stateData;

        std::ofstream o(statesFile);
        if (o.is_open()) {
            o << statesJson.dump(4);
            o.close();
            Logger::AddLog("Saved Camera & Active Objects config for state '%s'", stateName.c_str());
        } else {
            Logger::AddLog("[ERROR] Failed to save states.json config.");
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Create New State");
    static char newStateName[128] = "";
    ImGui::InputText("##NewStateName", newStateName, 128);
    ImGui::SameLine();
    if (ImGui::Button("Add State") && strlen(newStateName) > 0) {
        int nextId = 0;
        for(auto const& [id, name] : allStates) if(id >= nextId) nextId = id + 1;
        GameStateManager::RegisterState(nextId, newStateName);
        
        StateManager::RegisterState<StartScreen>(newStateName);
        memset(newStateName, 0, 128);
    }
    } 

    ImGui::End();
}

void EditorLayer::DrawViewport(Scene& scene, Camera& camera) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Viewport", nullptr, lockFlags);
    
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    
    bool showSplit = (selectedCube != -1 && selectedCube < scene.GetObjects().size() && 
                      scene.GetObjects()[selectedCube].hasCamera && 
                      scene.GetObjects()[selectedCube].camera.enabled);

    viewportWidth = (int)viewportPanelSize.x;
    viewportHeight = (int)viewportPanelSize.y;

    if (showSplit) {
        float splitRatio = 0.65f;
        viewportWidth = (int)(viewportPanelSize.x * splitRatio - 4.0f);
    }

    if (viewportTextureID != 0 && viewportWidth > 0 && viewportHeight > 0) {
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        
        if (showSplit) {
            float splitRatio = 0.65f;
            ImVec2 mainSize(viewportWidth, viewportHeight);
            ImVec2 camSize(viewportPanelSize.x * (1.0f - splitRatio), viewportPanelSize.y);
            
            ImGui::Image((ImTextureID)(intptr_t)viewportTextureID, mainSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::SameLine();
            
            auto& camObj = scene.GetObjects()[selectedCube].camera;
            unsigned int camTex = camObj.renderTexture;
            ImGui::BeginChild("CamPreviewSplit", camSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::TextColored(ImVec4(1,1,0,1), "Camera Preview");
            
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float aspect = (float)camObj.resolutionX / (float)camObj.resolutionY;
            float availAspect = avail.x / avail.y;
            ImVec2 imgSize;
            if (aspect > availAspect) {
                imgSize.x = avail.x;
                imgSize.y = avail.x / aspect;
            } else {
                imgSize.y = avail.y;
                imgSize.x = avail.y * aspect;
            }
            
            ImVec2 offset;
            offset.x = (avail.x - imgSize.x) * 0.5f;
            offset.y = (avail.y - imgSize.y) * 0.5f;
            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + offset.x, ImGui::GetCursorPosY() + offset.y));

            ImGui::Image((ImTextureID)(intptr_t)camTex, imgSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::EndChild();
        } else {
            ImGui::Image(
                (ImTextureID)(intptr_t)viewportTextureID,
                ImVec2(viewportWidth, viewportHeight),
                ImVec2(0, 1),  
                ImVec2(1, 0)   
            );
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_FILE")) {
                std::string modelPath((const char*)payload->Data, payload->DataSize - 1);
                Logger::AddLog("Importing model into scene: %s", modelPath.c_str());
                auto result = ModelImporter::Import(modelPath);
                if (result.success) {
                    auto& app = Application::Get();
                    
                    std::vector<Vertex> rawVerts;
                    std::vector<GLuint> rawIndices;
                    Mesh groupMesh(rawVerts, rawIndices, {});
                    std::string groupName = "Model: " + std::filesystem::path(modelPath).stem().string();
                    
                    int parentId = (int)app.GetScene()->GetObjects().size();
                    GameObject groupObj(std::move(groupMesh), groupName);
                    groupObj.isStatic = true;
                    app.GetScene()->AddObject(std::move(groupObj));
                    
                    int meshIdx = 0;
                    for (auto& meshData : result.meshes) {
                        Mesh mesh(meshData.vertices, meshData.indices, meshData.textures);
                        GameObject obj(std::move(mesh), meshData.name);
                        obj.material.albedo = meshData.albedo;
                        obj.modelPath = modelPath;
                        obj.meshIndex = meshIdx++;
                        obj.meshType = MeshType::Model;
                        obj.isStatic = true;
                        
                        if (!meshData.textures.empty()) {
                            obj.material.useTexture = true;
                            obj.material.diffuseTexture = meshData.textures[0].path;
                        }

                        obj.parentIndex = parentId;
                        app.GetScene()->AddObject(std::move(obj));
                    }
                    Logger::AddLog("Added model group: %s (%zu meshes)", groupName.c_str(), result.meshes.size());
                } else {
                    Logger::AddLog("[ERROR] Import failed: %s", result.error.c_str());
                }
            }
            ImGui::EndDragDropTarget();
        }
        
        
        if (Editor::isEditMode && (ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) && 
            !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) gizmoOp = 0; 
            if (ImGui::IsKeyPressed(ImGuiKey_E)) gizmoOp = 1; 
            if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOp = 2; 
        }
        
        
        if (uiEditMode && Editor::isEditMode) {
            std::string activeScreenName = GameStateManager::GetStateName(m_PreviewState);

            auto& elements = UICreationEngine::GetElements();
            for (int i = 0; i < (int)elements.size(); i++) {
                auto& el = elements[i];
                
                
                if (el.screenName != activeScreenName) continue;
                
                float centerX = viewportWidth * el.anchorMin.x;
                float centerY = viewportHeight * el.anchorMin.y;
                float finalX = centerX + el.position.x - (el.size.x * el.pivot.x);
                float finalY = centerY + el.position.y - (el.size.y * el.pivot.y);

                
                ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + finalX, cursorPos.y + finalY));
                std::string id = "##UIEl" + std::to_string(i);
                ImGui::PushID(i);
                
                ImVec2 size = ImVec2(el.size.x, el.size.y);
                if (size.x < 10) size.x = 10;
                if (size.y < 10) size.y = 10;

                ImGui::InvisibleButton(id.c_str(), size);
                
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    el.position.x += delta.x;
                    el.position.y += delta.y;
                    selectedUIElement = i;
                }
                
                if (ImGui::IsItemClicked()) {
                    selectedUIElement = i;
                }

                
                if (selectedUIElement == i) {
                    float edgeWidth = 6.0f;
                    ImVec2 tl = ImVec2(cursorPos.x + finalX, cursorPos.y + finalY);
                    ImVec2 br = ImVec2(cursorPos.x + finalX + el.size.x, cursorPos.y + finalY + el.size.y);
                    ImDrawList* drawList = ImGui::GetWindowDrawList();

                    
                    auto handleEdge = [&](const char* id, ImVec2 hpos, ImVec2 hsize, int axis, int side) {
                        ImGui::SetCursorScreenPos(hpos);
                        ImGui::InvisibleButton(id, hsize);
                        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                            float delta = (axis == 0) ? ImGui::GetIO().MouseDelta.x : ImGui::GetIO().MouseDelta.y;
                            if (side == 1) { 
                                if (axis == 0) { el.size.x += delta; el.position.x += delta * el.pivot.x; }
                                else { el.size.y += delta; el.position.y += delta * el.pivot.y; }
                            } else { 
                                if (axis == 0) { el.size.x -= delta; el.position.x += delta * (1.0f - el.pivot.x); }
                                else { el.size.y -= delta; el.position.y += delta * (1.0f - el.pivot.y); }
                            }
                            if (el.size.x < 5) el.size.x = 5;
                            if (el.size.y < 5) el.size.y = 5;
                        }
                        if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
                             drawList->AddRectFilled(hpos, ImVec2(hpos.x + hsize.x, hpos.y + hsize.y), ImColor(1.0f, 1.0f, 0.0f, 0.4f));
                        }
                    };

                    
                    handleEdge("##RE", ImVec2(br.x - edgeWidth / 2, tl.y), ImVec2(edgeWidth, el.size.y), 0, 1);
                    
                    handleEdge("##BE", ImVec2(tl.x, br.y - edgeWidth / 2), ImVec2(el.size.x, edgeWidth), 1, 1);
                    
                    handleEdge("##LE", ImVec2(tl.x - edgeWidth / 2, tl.y), ImVec2(edgeWidth, el.size.y), 0, -1);
                    
                    handleEdge("##TE", ImVec2(tl.x, tl.y - edgeWidth / 2), ImVec2(el.size.x, edgeWidth), 1, -1);
                }

                
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec4 color = (selectedUIElement == i) ? ImVec4(1, 1, 0, 1) : ImVec4(1, 1, 1, 0.5f);
                drawList->AddRect(
                    ImVec2(cursorPos.x + finalX, cursorPos.y + finalY),
                    ImVec2(cursorPos.x + finalX + el.size.x, cursorPos.y + finalY + el.size.y),
                    ImColor(color)
                );
                
                ImGui::PopID();
            }
        }

        
        
        std::string activeScreenName;
        if (!Editor::isEditMode) {
            
            if (m_PlayModeActiveScreen.empty()) {
                m_PlayModeActiveScreen = GameStateManager::GetStateName(m_PreviewState);
            }
            activeScreenName = m_PlayModeActiveScreen;
        } else {
            activeScreenName = GameStateManager::GetStateName(m_PreviewState);
        }
        std::vector<UIElement> filteredElements;
        auto& allElements = UICreationEngine::GetElements();
        for(const auto& el : allElements) {
            if (el.screenName == activeScreenName) filteredElements.push_back(el);
        }
        
        if (!Editor::isEditMode) {
            PlayMode::RenderAndHandleClicks(filteredElements, viewportWidth, viewportHeight, cursorPos, m_PlayModeActiveScreen);
        } else {
            
            UIManager::Render(filteredElements, glm::vec2(viewportWidth, viewportHeight), glm::vec2(cursorPos.x, cursorPos.y));
        }
        
        bool gizmoUsing = ImGuizmo::IsUsing();
        bool gizmoOver = ImGuizmo::IsOver();
        
        if (Editor::isEditMode && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) 
            && !gizmoUsing && !gizmoOver) {
            
            ImVec2 mousePos = ImGui::GetMousePos();
            float mx = mousePos.x - cursorPos.x;
            float my = mousePos.y - cursorPos.y;
            
            
            float ndcX = (2.0f * mx) / viewportWidth - 1.0f;
            float ndcY = 1.0f - (2.0f * my) / viewportHeight; 
            
            
            glm::mat4 invProj = glm::inverse(camera.GetProjectionMatrix());
            glm::mat4 invView = glm::inverse(camera.GetViewMatrix());
            
            glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 rayEye = invProj * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
            glm::vec3 rayDir = glm::normalize(glm::vec3(invView * rayEye));
            glm::vec3 rayOrigin = camera.Position;
            
            
            int closestIdx = -1;
            float closestDist = 999999.0f;
            
            auto& objects = scene.GetObjects();
            for (int i = 0; i < (int)objects.size(); ++i) {
                auto& obj = objects[i];
                
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, obj.position);
                model *= glm::mat4_cast(obj.rotation);
                model = glm::scale(model, obj.scale);
                
                float dist;
                if (obj.mesh.Intersect(rayOrigin, rayDir, model, dist)) {
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestIdx = i;
                    }
                }
            }
            
            
            auto& pointLights = scene.GetPointLights();
            for (int i = 0; i < pointLights.size(); ++i) {
                if (!pointLights[i].enabled) continue;
                
                glm::vec3 lightPos = pointLights[i].position;
                glm::vec4 clip = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(lightPos, 1.0f);
                if (clip.w > 0.0f) {
                     glm::vec3 ndc = glm::vec3(clip) / clip.w;
                     float screenX = (ndc.x + 1.0f) * 0.5f * viewportWidth;
                     float screenY = (1.0f - ndc.y) * 0.5f * viewportHeight;
                     
                     float dx = mx - screenX;
                     float dy = my - screenY;
                     if (dx*dx + dy*dy < 15.0f * 15.0f) {
                         closestIdx = -1;
                         selectedPointLightIndex = i;
                         selectedCube = -1; selectedMesh = -1; isLightSelected = false;
                         Logger::AddLog("Viewport: Selected Point Light %d", i);
                         break; 
                     }
                }
            }
            
            
            
            if (closestIdx >= 0) {
                selectedCube = closestIdx;
                selectedMesh = -1;
                isLightSelected = false;
                selectedPointLightIndex = -1;
                Logger::AddLog("Viewport: Selected %s", objects[closestIdx].name.c_str());
            } else if (selectedPointLightIndex == -1) {
                
                selectedCube = -1;
                isLightSelected = false;
            }
        }
        
        
        {
            auto& pointLights = scene.GetPointLights();
            for (int i = 0; i < pointLights.size(); ++i) {
                if (!pointLights[i].enabled) continue;
                
                glm::vec3 lightPos = pointLights[i].position;
                glm::vec4 clip = camera.GetProjectionMatrix() * camera.GetViewMatrix() * glm::vec4(lightPos, 1.0f);
                if (clip.w > 0.0f) {
                     glm::vec3 ndc = glm::vec3(clip) / clip.w;
                     float screenX = cursorPos.x + (ndc.x + 1.0f) * 0.5f * viewportWidth;
                     float screenY = cursorPos.y + (1.0f - ndc.y) * 0.5f * viewportHeight;
                     
                     ImDrawList* drawList = ImGui::GetWindowDrawList();
                     drawList->AddCircleFilled(ImVec2(screenX, screenY), 8.0f, IM_COL32(255, 200, 50, 255));
                     if (selectedPointLightIndex == i)
                        drawList->AddCircle(ImVec2(screenX, screenY), 10.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
                }
            }
        }
        
        
        
       
        
        
        if (selectedCube >= 0) {
            auto& objects = scene.GetObjects();
            if (selectedCube < (int)objects.size()) {
                auto& obj = objects[selectedCube];
                
                
                glm::mat4 view = camera.GetViewMatrix();
                glm::mat4 projection = camera.GetProjectionMatrix();
                
                glm::mat4 worldMatrix = scene.GetGlobalTransform(selectedCube);
                
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportWidth, viewportHeight);
                
                ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
                if (gizmoOp == 1) op = ImGuizmo::ROTATE;
                if (gizmoOp == 2) op = ImGuizmo::SCALE;
                ImGuizmo::MODE mode = gizmoLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
                
                if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, mode, glm::value_ptr(worldMatrix))) {
                    glm::mat4 localMatrix = worldMatrix;
                    if (obj.parentIndex != -1 && obj.parentIndex < scene.GetObjects().size()) {
                        glm::mat4 parentWorld = scene.GetGlobalTransform(obj.parentIndex);
                        localMatrix = glm::inverse(parentWorld) * worldMatrix;
                    }

                    glm::vec3 skew; glm::vec4 perspective;
                    glm::decompose(localMatrix, obj.scale, obj.rotation, obj.position, skew, perspective);
                }
            }
        }
        
        
        if (selectedPointLightIndex >= 0) {
             auto& pl = scene.GetPointLights();
             if (selectedPointLightIndex < (int)pl.size()) {
                 auto& light = pl[selectedPointLightIndex];
                 
                 glm::mat4 view = camera.GetViewMatrix();
                 glm::mat4 projection = camera.GetProjectionMatrix();
                 glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), light.position);
                 
                 ImGuizmo::SetOrthographic(false);
                 ImGuizmo::SetDrawlist();
                 ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportWidth, viewportHeight);
                 
                 ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
                 ImGuizmo::MODE mode = ImGuizmo::WORLD;
                 
                 if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, mode, glm::value_ptr(modelMatrix))) {
                     glm::vec3 skew; glm::vec4 perspective; glm::vec3 newPos, newScale; glm::quat newRot;
                     glm::decompose(modelMatrix, newScale, newRot, newPos, skew, perspective);
                     light.position = newPos;
                 }
             }
        }
        
        
        ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 8, cursorPos.y + 8));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
        
        auto toolButton = [&](const char* label, int op) {
            bool active = (gizmoOp == op);
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
            if (ImGui::SmallButton(label)) gizmoOp = op;
            if (active) ImGui::PopStyleColor();
            ImGui::SameLine();
        };
        
        toolButton("W Move", 0);
        toolButton("E Rotate", 1);
        toolButton("R Scale", 2);
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        if (ImGui::SmallButton(gizmoLocal ? "Local" : "World")) {
            gizmoLocal = !gizmoLocal;
        }
        
        ImGui::PopStyleVar(2);
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::DrawContentBrowser() {
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Content Browser", nullptr, lockFlags);
    
    if (m_ProjectRoot.empty()) {
        ImGui::Text("No project loaded.");
        ImGui::End();
        return;
    }

    
    if (ImGui::Button("Home")) {
        m_CurrentContentPath = m_ProjectRoot;
    }
    
    
    if (m_CurrentContentPath != m_ProjectRoot) {
        ImGui::SameLine();
        if (ImGui::Button("^ Up")) {
            auto parent = std::filesystem::path(m_CurrentContentPath).parent_path().string();
            if (!parent.empty()) {
                m_CurrentContentPath = parent;
            }
        }
    }
    
    ImGui::SameLine();
    ImGui::Text("> %s", m_CurrentContentPath.c_str());
    ImGui::Separator();
    
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) && m_CurrentContentPath != m_ProjectRoot) {
        if (ImGui::IsKeyPressed(ImGuiKey_Backspace) || ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            m_CurrentContentPath = std::filesystem::path(m_CurrentContentPath).parent_path().string();
        }
    }

    static float leftWidth = 200.0f;
    ImGui::BeginChild("TreeChild", ImVec2(leftWidth, 0), true);
    DrawContentBrowserTree(m_ProjectRoot);
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginChild("GridChild", ImVec2(0, 0), true);
    DrawContentBrowserGrid();
    ImGui::EndChild();
    
    ImGui::End();
}

void EditorLayer::DrawContentBrowserTree(const std::string& path) {
    namespace fs = std::filesystem;
    try {
        for (auto& entry : fs::directory_iterator(path)) {
            if (!entry.is_directory()) continue;
            
            std::string name = entry.path().filename().string();
            ImGuiTreeNodeFlags flags = ((m_CurrentContentPath == entry.path().string()) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            
            bool opened = ImGui::TreeNodeEx(name.c_str(), flags);
            if (ImGui::IsItemClicked()) {
                m_CurrentContentPath = entry.path().string();
            }
            
            if (opened) {
                DrawContentBrowserTree(entry.path().string());
                ImGui::TreePop();
            }
        }
    } catch (...) {}
}

void EditorLayer::DrawContentBrowserGrid() {
    namespace fs = std::filesystem;
    
    float padding = 16.0f;
    float cellSize = 96.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = (int)(panelWidth / (cellSize + padding));
    if (columns < 1) columns = 1;

    
    if (ImGui::BeginPopupContextWindow("ContentBrowserPopup", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("New Script (.cpp)")) {
            ImGui::OpenPopup("NewScriptPopup_Main");
        }
        if (ImGui::MenuItem("New Audio Script (UI Trigger)")) {
            ImGui::OpenPopup("NewAudioScriptPopup");
        }
        if (ImGui::MenuItem("New Shader Pair (GLSL)")) {
            ImGui::OpenPopup("NewShaderPopup_Main");
        }
        if (ImGui::MenuItem("New Folder")) {
            ImGui::OpenPopup("NewFolderPopup");
        }
        ImGui::EndPopup();
    }

    
    static char newScriptName[128] = "MyScript";
    static bool openNewScriptPopup = false;
    if (ImGui::BeginPopup("NewScriptPopup_Main")) {
        ImGui::Text("Script Name:");
        ImGui::InputText("##ScriptName", newScriptName, 128);
        if (ImGui::Button("Create Movement Script")) {
            std::string scriptPath = m_CurrentContentPath + "/" + std::string(newScriptName) + ".cpp";
            std::ofstream file(scriptPath);
            if (file.is_open()) {
                file << "#include \"Scene/Behavior.h\"\n";
                file << "#include \"Scene/BehaviorRegistry.h\"\n";
                file << "#include \"Scene/Scene.h\"\n";
                file << "#include \"Core/InputManager.h\"\n";
                file << "#include <GLFW/glfw3.h>\n";
                file << "#include <glm/glm.hpp>\n";
                file << "#include <glm/gtc/quaternion.hpp>\n";
                file << "#include <iostream>\n\n";
                file << "class " << newScriptName << " : public Behavior {\n";
                file << "public:\n";
                file << "    float speed = 5.0f;\n\n";
                file << "    void OnStart() override {\n";
                file << "        std::cout << \"" << newScriptName << " started on: \" << gameObject->name << std::endl;\n";
                file << "    }\n\n";
                file << "    void OnUpdate(float dt) override {\n";
                file << "        if (!gameObject) return;\n\n";
                file << "        glm::vec3 move(0.0f);\n";
                file << "        if (InputManager::IsKeyPressed(GLFW_KEY_W)) move.z -= 1.0f;\n";
                file << "        if (InputManager::IsKeyPressed(GLFW_KEY_S)) move.z += 1.0f;\n";
                file << "        if (InputManager::IsKeyPressed(GLFW_KEY_A)) move.x -= 1.0f;\n";
                file << "        if (InputManager::IsKeyPressed(GLFW_KEY_D)) move.x += 1.0f;\n\n";
                file << "        if (glm::length(move) > 0.1f) {\n";
                file << "            move = glm::normalize(move) * speed * dt;\n";
                file << "            gameObject->position += move;\n";
                file << "        }\n";
                file << "    }\n";
                file << "};\n\n";
                file << "REGISTER_BEHAVIOR(" << newScriptName << ")\n";
                file.close();
                Logger::AddLog("Created movement script: %s", scriptPath.c_str());
            }
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
    }
    
    static char audioScriptName[128] = "MyAudioScript";
    static char targetBtnName[128] = "PlayBtn";
    static char targetObjName[128] = "AudioSource";
    
    if (ImGui::BeginPopup("NewAudioScriptPopup")) {
        ImGui::Text("Script Name:");
        ImGui::InputText("##AudioScriptName", audioScriptName, 128);
        ImGui::Text("Trigger UI Button Name:");
        ImGui::InputText("##TargetBtnName", targetBtnName, 128);
        ImGui::Text("Target Audio Object Name:");
        ImGui::InputText("##TargetObjName", targetObjName, 128);

        if (ImGui::Button("Create Script")) {
            std::string scriptPath = m_CurrentContentPath + "/" + std::string(audioScriptName) + ".cpp";
            std::ofstream file(scriptPath);
            if (file.is_open()) {
                file << "#include \"Scene/Behavior.h\"\n";
                file << "#include \"Scene/BehaviorRegistry.h\"\n";
                file << "#include \"Scene/Scene.h\"\n";
                file << "#include \"Core/InputManager.h\"\n";
                file << "#include \"AudioEngine/AudioEngine.h\"\n";
                file << "#include \"Core/Application.h\"\n";
                file << "#include <GLFW/glfw3.h>\n";
                file << "#include <iostream>\n\n";
                file << "class " << audioScriptName << " : public Behavior {\n";
                file << "public:\n";
                file << "    void OnStart() override {\n";
                file << "        std::cout << \"" << audioScriptName << " initialized.\\n\";\n";
                file << "    }\n\n";
                file << "    void OnUpdate(float dt) override {\n";
                file << "        if (!gameObject) return;\n\n";
                file << "        if (InputManager::IsUIButtonClicked(\"" << targetBtnName << "\")) {\n";
                file << "            if (Scene* scene = Application::Get().GetScene()) {\n";
                file << "               for (auto& obj : scene->GetObjects()) {\n";
                file << "                   if (obj.name == \"" << targetObjName << "\" && obj.hasAudio) {\n";
                file << "                       if (!obj.audio.playing) {\n";
                file << "                           obj.audio.playing = true;\n";
                file << "                           AudioEngine::PlayObjectAudio(obj);\n";
                file << "                           std::cout << \"Playing Target Audio!\\n\";\n";
                file << "                       }\n";
                file << "                       break;\n";
                file << "                   }\n";
                file << "               }\n";
                file << "            }\n";
                file << "        }\n";
                file << "    }\n";
                file << "};\n\n";
                file << "REGISTER_BEHAVIOR(" << audioScriptName << ")\n";
                file.close();
                Logger::AddLog("Created auto-playing audio script: %s", scriptPath.c_str());
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    static char newShaderName[128] = "MyShader";
    if (ImGui::BeginPopup("NewShaderPopup_Main")) {
        ImGui::Text("Shader Name:");
        ImGui::InputText("##ShaderName", newShaderName, 128);
        if (ImGui::Button("Create Pair")) {
            std::string vertPath = m_CurrentContentPath + "/" + std::string(newShaderName) + ".vert";
            std::string fragPath = m_CurrentContentPath + "/" + std::string(newShaderName) + ".frag";
            std::ofstream vfile(vertPath);
            if (vfile.is_open()) {
                vfile << "#version 330 core\nlayout (location = 0) in vec3 aPos;\nuniform mat4 model;\nuniform mat4 view;\nuniform mat4 projection;\nvoid main() {\n    gl_Position = projection * view * model * vec4(aPos, 1.0);\n}\n";
                vfile.close();
            }
            std::ofstream ffile(fragPath);
            if (ffile.is_open()) {
                ffile << "#version 330 core\nout vec4 FragColor;\nvoid main() {\n    FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n}\n";
                ffile.close();
            }
            Logger::AddLog("Created shader pair: %s", newShaderName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    static char newFolderName[128] = "NewFolder";
    if (ImGui::BeginPopup("NewFolderPopup_Main")) {
        ImGui::Text("Folder Name:");
        ImGui::InputText("##FolderName", newFolderName, 128);
        if (ImGui::Button("Create")) {
            std::string folderPath = m_CurrentContentPath + "/" + std::string(newFolderName);
            fs::create_directories(folderPath);
            Logger::AddLog("Created folder: %s", folderPath.c_str());
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Columns(columns, nullptr, false);
    
    static std::string pendingDeletePath;
    static std::string clipboardContentPath;
    
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
            if (!m_SelectedContentFile.empty() && std::filesystem::exists(m_SelectedContentFile)) {
                clipboardContentPath = m_SelectedContentFile;
                Logger::AddLog("Copied file path: %s", clipboardContentPath.c_str());
            }
        }
        if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V)) {
            if (!clipboardContentPath.empty() && std::filesystem::exists(clipboardContentPath)) {
                try {
                    auto target = std::filesystem::path(m_CurrentContentPath) / std::filesystem::path(clipboardContentPath).filename();
                    if (std::filesystem::exists(target)) {
                        std::string stem = target.stem().string();
                        std::string ext = target.extension().string();
                        int counter = 1;
                        while(std::filesystem::exists(target)) {
                            target = std::filesystem::path(m_CurrentContentPath) / (stem + "_" + std::to_string(counter) + ext);
                            counter++;
                        }
                    }
                    std::filesystem::copy(clipboardContentPath, target, std::filesystem::copy_options::recursive);
                    Logger::AddLog("Pasted file/folder layout to: %s", target.string().c_str());
                } catch(const std::exception& e) {
                    Logger::AddLog("[ERROR] Paste File Failed: %s", e.what());
                }
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !m_SelectedContentFile.empty()) {
            pendingDeletePath = m_SelectedContentFile;
            ImGui::OpenPopup("ConfirmDelete");
        }
    }
    
    try {
        for (auto& entry : fs::directory_iterator(m_CurrentContentPath)) {
            std::string filename = entry.path().filename().string();
            std::string ext = entry.path().extension().string();
            
            ImGui::PushID(filename.c_str());
            
            bool isImage = (ext == ".png" || ext == ".jpg" || ext == ".jpeg");
            GLuint thumbID = 0;
            
            if (isImage) {
                std::string thumbKey = "thumb_" + entry.path().string();
                if (!ResourceManager::HasTexture(thumbKey)) {
                    ResourceManager::LoadTexture(thumbKey, entry.path().string().c_str(), "diffuse", 0);
                }
                if (ResourceManager::HasTexture(thumbKey)) {
                    thumbID = ResourceManager::GetTexture(thumbKey).ID;
                }
            }

            ImVec4 iconColor;
            if (entry.is_directory()) {
                iconColor = ImVec4(1.0f, 0.85f, 0.4f, 1.0f);  
            } else if (ext == ".cpp" || ext == ".h") {
                iconColor = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);   
            } else if (ext == ".vert" || ext == ".frag") {
                iconColor = ImVec4(0.1f, 0.8f, 0.9f, 1.0f);   
            } else if (ext == ".scene") {
                iconColor = ImVec4(0.3f, 0.8f, 0.4f, 1.0f);   
            } else if (ext == ".json") {
                iconColor = ImVec4(0.9f, 0.6f, 0.2f, 1.0f);   
            } else if (isImage && thumbID == 0) {
                iconColor = ImVec4(0.8f, 0.3f, 0.8f, 1.0f);   
            } else if (ModelImporter::IsModelFile(ext)) {
                iconColor = ImVec4(0.95f, 0.4f, 0.7f, 1.0f);  
            } else {
                iconColor = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);   
            }
            
            if (thumbID != 0) {
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
#if IMGUI_VERSION_NUM >= 18900
                if (ImGui::ImageButton(filename.c_str(), (void*)(intptr_t)thumbID, ImVec2(cellSize, cellSize))) {
                    m_SelectedContentFile = entry.path().string();
                }
#else
                if (ImGui::ImageButton((void*)(intptr_t)thumbID, ImVec2(cellSize, cellSize))) {
                    m_SelectedContentFile = entry.path().string();
                }
#endif
                ImGui::PopStyleColor();
            } else {
                
                ImGui::PushStyleColor(ImGuiCol_Button, iconColor);
                if (ImGui::Button(filename.c_str(), ImVec2(cellSize, cellSize))) {
                    m_SelectedContentFile = entry.path().string();
                }
                ImGui::PopStyleColor();
            }

            
            if (!entry.is_directory() && ModelImporter::IsModelFile(ext)) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string path = entry.path().string();
                    ImGui::SetDragDropPayload("MODEL_FILE", path.c_str(), path.size() + 1);
                    ImGui::Text("Import: %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }

            
            if (!entry.is_directory() && (ext == ".png" || ext == ".jpg" || ext == ".jpeg")) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string path = entry.path().string();
                    ImGui::SetDragDropPayload("TEXTURE_FILE", path.c_str(), path.size() + 1);
                    ImGui::Text("Texture: %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            
            
            if (!entry.is_directory() && (ext == ".vert" || ext == ".frag")) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string path = entry.path().string();
                    ImGui::SetDragDropPayload("SHADER_FILE", path.c_str(), path.size() + 1);
                    ImGui::Text("Shader: %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }

            
            bool isAudioFile = (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac");
            if (!entry.is_directory() && isAudioFile) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string path = entry.path().string();
                    ImGui::SetDragDropPayload("AUDIO_FILE", path.c_str(), path.size() + 1);
                    ImGui::Text("Audio: %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            
            bool isVideoFile = (ext == ".mp4" || ext == ".avi" || ext == ".mkv" || ext == ".mov");
            if (!entry.is_directory() && (isImage || isVideoFile)) {
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string path = entry.path().string();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", path.c_str(), path.size() + 1);
                    ImGui::Text("File: %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (entry.is_directory()) {
                    m_CurrentContentPath = entry.path().string();
                } else if (ext == ".vert" || ext == ".frag") {
                    m_EditingShaderPath = entry.path().string();
                    std::ifstream t(m_EditingShaderPath);
                    if (t.is_open()) {
                        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
                        strncpy(m_ShaderEditorBuffer, str.c_str(), sizeof(m_ShaderEditorBuffer) - 1);
                        m_ShaderEditorBuffer[sizeof(m_ShaderEditorBuffer) - 1] = '\0';
                        Logger::AddLog("Opened shader for editing: %s", filename.c_str());
                    }
                } else if (ext == ".scene") {
                    auto& app = Application::Get();
                    app.GetScene()->Load(entry.path().string());
                    Logger::AddLog("Loaded scene: %s", filename.c_str());
                } else if (ModelImporter::IsModelFile(ext)) {
                    auto result = ModelImporter::Import(entry.path().string());
                    if (result.success) {
                        auto& app = Application::Get();
                        
                        std::vector<Vertex> rawVerts;
                        std::vector<GLuint> rawIndices;
                        Mesh groupMesh(rawVerts, rawIndices, {});
                        std::string groupName = "Model: " + std::filesystem::path(entry.path().string()).stem().string();
                        
                        int parentId = (int)app.GetScene()->GetObjects().size();
                        GameObject groupObj(std::move(groupMesh), groupName);
                        groupObj.isStatic = true;
                        app.GetScene()->AddObject(std::move(groupObj));

                        int meshIdx = 0;
                        for (auto& meshData : result.meshes) {
                            Mesh mesh(meshData.vertices, meshData.indices, meshData.textures);
                            GameObject obj(std::move(mesh), meshData.name);
                            obj.material.albedo = meshData.albedo;
                            obj.modelPath = entry.path().string();
                            obj.meshIndex = meshIdx++;
                            obj.meshType = MeshType::Model;
                            obj.isStatic = true;
                            
                            
                            if (!meshData.textures.empty()) {
                                obj.material.useTexture = true;
                                obj.material.diffuseTexture = meshData.textures[0].path;
                            } else {
                                obj.material.useTexture = false;
                            }
                            
                            obj.parentIndex = parentId;
                            app.GetScene()->AddObject(std::move(obj));
                        }
                        Logger::AddLog("Imported model group from %s (%zu meshes)", filename.c_str(), result.meshes.size());
                    } else {
                        Logger::AddLog("[ERROR] Import failed: %s", result.error.c_str());
                    }
                }
            }
            
            
            if (ImGui::BeginPopupContextItem()) {
                if (entry.is_directory()) {
                    if (ImGui::MenuItem("Open Folder")) {
                        m_CurrentContentPath = entry.path().string();
                    }
                }
                if (ext == ".scene" && ImGui::MenuItem("Load Scene")) {
                    auto& app = Application::Get();
                    app.GetScene()->Load(entry.path().string());
                    Logger::AddLog("Loaded scene: %s", filename.c_str());
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Copy")) {
                    clipboardContentPath = entry.path().string();
                    Logger::AddLog("Copied: %s", clipboardContentPath.c_str());
                }
                if (ImGui::MenuItem("Paste Here")) {
                    if (!clipboardContentPath.empty() && std::filesystem::exists(clipboardContentPath)) {
                        try {
                            
                            std::filesystem::path pasteDest = entry.is_directory() ? entry.path() : std::filesystem::path(m_CurrentContentPath);
                            auto target = pasteDest / std::filesystem::path(clipboardContentPath).filename();
                            if (std::filesystem::exists(target)) {
                                std::string stem = target.stem().string();
                                std::string ext = target.extension().string();
                                int counter = 1;
                                while(std::filesystem::exists(target)) {
                                    target = pasteDest / (stem + "_" + std::to_string(counter) + ext);
                                    counter++;
                                }
                            }
                            std::filesystem::copy(clipboardContentPath, target, std::filesystem::copy_options::recursive);
                            Logger::AddLog("Pasted file/folder to: %s", target.string().c_str());
                        } catch(const std::exception& e) {
                            Logger::AddLog("[ERROR] Paste File Failed: %s", e.what());
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    pendingDeletePath = entry.path().string();
                    ImGui::OpenPopup("ConfirmDelete");
                }
                ImGui::EndPopup();
            }
            
            ImGui::TextWrapped("%s", filename.c_str());
            
            ImGui::NextColumn();
            ImGui::PopID();
        }
    } catch (...) {}
    
    ImGui::Columns(1);

    
    ImGui::Separator();
    if (ImGui::Button("+ New Script")) {
        ImGui::OpenPopup("NewScriptPopup_Main");
    }
    ImGui::SameLine();
    if (ImGui::Button("+ New Shader")) {
        ImGui::OpenPopup("NewShaderPopup_Main");
    }
    ImGui::SameLine();
    if (ImGui::Button("+ New Folder")) {
        ImGui::OpenPopup("NewFolderPopup_Main");
    }

    
    if (ImGui::BeginPopupModal("ConfirmDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete '%s'?", fs::path(pendingDeletePath).filename().c_str());
        ImGui::Text("This cannot be undone!");
        ImGui::Separator();
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            try {
                fs::remove_all(pendingDeletePath);
                Logger::AddLog("Deleted: %s", pendingDeletePath.c_str());
            } catch (const std::exception& e) {
                Logger::AddLog("[ERROR] Failed to delete: %s", e.what());
            }
            pendingDeletePath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            pendingDeletePath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
void EditorLayer::DrawUIEditor() {
    static char filter[128] = "";
    ImGui::Checkbox("UI Edit Mode", &uiEditMode);
    ImGui::Separator();

    std::string activeScreenName = GameStateManager::GetStateName(m_PreviewState);

    if (ImGui::Button("Add Button")) {
        UIElement btn;
        btn.name = "Button_" + std::to_string(UICreationEngine::GetElements().size());
        btn.type = UIElementType::BUTTON;
        btn.text = "Click Me";
        btn.screenName = activeScreenName;
        UICreationEngine::AddElement(btn);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Text")) {
        UIElement txt;
        txt.name = "Text_" + std::to_string(UICreationEngine::GetElements().size());
        txt.type = UIElementType::TEXT;
        txt.text = "Hello UI";
        txt.screenName = activeScreenName;
        UICreationEngine::AddElement(txt);
    }

    ImGui::Separator();
    
    ImGui::InputText("Screen Filter", filter, 128);
    
    auto& elements = UICreationEngine::GetElements();
    for (int i = 0; i < (int)elements.size(); i++) {
        if (strlen(filter) > 0 && elements[i].screenName.find(filter) == std::string::npos) continue;
        
        std::string label = (elements[i].screenName + "/" + elements[i].name) + "##" + std::to_string(i);
        if (ImGui::Selectable(label.c_str(), selectedUIElement == i)) {
            selectedUIElement = i;
        }
    }

    if (selectedUIElement != -1 && selectedUIElement < (int)elements.size()) {
        ImGui::Separator();
        auto& el = elements[selectedUIElement];
        
        char nameBuf[256]; 
        memset(nameBuf, 0, 256);
        strncpy(nameBuf, el.name.c_str(), 255);
        if (ImGui::InputText("Name", nameBuf, 256)) el.name = nameBuf;
        
        char textBuf[256]; 
        memset(textBuf, 0, 256);
        strncpy(textBuf, el.text.c_str(), 255);
        if (ImGui::InputText("Text", textBuf, 256)) el.text = textBuf;

        auto& allStates = GameStateManager::GetAllStates();
        std::vector<const char*> screenOptions;
        for (auto const& [id, name] : allStates) {
            screenOptions.push_back(name.c_str());
        }

        int currentScreenIdx = 0;
        for (int i = 0; i < screenOptions.size(); i++) {
            if (el.screenName == screenOptions[i]) currentScreenIdx = i;
        }
        if (ImGui::Combo("Screen", &currentScreenIdx, screenOptions.data(), (int)screenOptions.size())) {
            el.screenName = screenOptions[currentScreenIdx];
        }

        ImGui::DragFloat2("Pos Offset", &el.position.x);
        ImGui::DragFloat2("Size", &el.size.x);
        
        ImGui::Separator();
        ImGui::DragFloat2("Anchor Min", &el.anchorMin.x, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat2("Anchor Max", &el.anchorMax.x, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat2("Pivot", &el.pivot.x, 0.01f, 0.0f, 1.0f);
        ImGui::Separator();

        ImGui::ColorEdit4("Color", &el.color.r);
        
        if (el.type == UIElementType::BUTTON) {
            ImGui::Separator();
            ImGui::Text("Button Scripting");
            
            const char* actionOptions[] = { "None", "ChangeState", "PushState", "PopState", "PlayAudio", "ToggleVideo", "PlayVideo", "PauseVideo" };
            int currentActionIdx = 0;
            for (int i = 0; i < 8; i++) {
                if (el.actionType == actionOptions[i]) currentActionIdx = i;
            }
            if (ImGui::Combo("Script Action", &currentActionIdx, actionOptions, 8)) {
                el.actionType = actionOptions[currentActionIdx];
            }
            
            if (el.actionType == "ChangeState" || el.actionType == "PushState") {
                auto states = GameStateManager::GetRegisteredStateNames();
                if (!states.empty()) {
                    int currentStateIdx = 0;
                    for (size_t i = 0; i < states.size(); i++) {
                        if (el.targetState == states[i]) currentStateIdx = i;
                    }
                    
                    std::vector<const char*> stateNamesCstrs;
                    for (const auto& s : states) stateNamesCstrs.push_back(s.c_str());
                    
                    if (ImGui::Combo("Target State", &currentStateIdx, stateNamesCstrs.data(), stateNamesCstrs.size())) {
                        el.targetState = states[currentStateIdx];
                    }
                }
            } else if (el.actionType == "PlayAudio") {
                if (Scene* scene = Application::Get().GetScene()) {
                    auto& objects = scene->GetObjects();
                    std::vector<const char*> audioNames;
                    int currentAudioIdx = 0;
                    int audioIndex = 0;
                    for (int i = 0; i < (int)objects.size(); i++) {
                        if (objects[i].hasAudio) {
                            if (el.targetAudioObject == objects[i].name) currentAudioIdx = audioIndex;
                            audioNames.push_back(objects[i].name.c_str());
                            audioIndex++;
                        }
                    }
                    if (!audioNames.empty()) {
                        if (el.targetAudioObject.empty()) {
                            el.targetAudioObject = audioNames[0];
                        }
                        
                        if (ImGui::Combo("Target Audio", &currentAudioIdx, audioNames.data(), audioNames.size())) {
                            el.targetAudioObject = audioNames[currentAudioIdx];
                        }
                    } else {
                        ImGui::TextDisabled("No objects with Audio component found.");
                    }
                }
            } else if (el.actionType == "ToggleVideo" || el.actionType == "PlayVideo" || el.actionType == "PauseVideo") {
                if (Scene* scene = Application::Get().GetScene()) {
                    auto& objects = scene->GetObjects();
                    std::vector<const char*> videoNames;
                    int currentVideoIdx = -1;
                    int videoIndex = 0;
                    for (int i = 0; i < (int)objects.size(); i++) {
                        if (objects[i].hasScreen && objects[i].screen.type == ScreenType::Video) {
                            if (el.targetVideoObject == objects[i].name) currentVideoIdx = videoIndex;
                            videoNames.push_back(objects[i].name.c_str());
                            videoIndex++;
                        }
                    }
                    if (!videoNames.empty()) {
                        if (currentVideoIdx == -1) {
                            currentVideoIdx = 0;
                            el.targetVideoObject = videoNames[0];
                        }
                        if (ImGui::Combo("Target Video", &currentVideoIdx, videoNames.data(), (int)videoNames.size())) {
                            el.targetVideoObject = videoNames[currentVideoIdx];
                        }
                    } else {
                        ImGui::TextDisabled("No objects with Video screen found.");
                    }
                }
            }
        }

        
        ImGui::Separator();
        if (ImGui::Button("Delete Element")) {
            elements.erase(elements.begin() + selectedUIElement);
            selectedUIElement = -1;
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Save UI Layout")) {
        std::string projRoot = Application::Get().GetProjectRoot();
        if (projRoot.empty()) projRoot = ".";
        std::string layoutPath = (std::filesystem::path(projRoot) / "ui_layout.json").string();
        UICreationEngine::SaveLayout(layoutPath);
        Logger::AddLog("Saved UI Layout to: %s", layoutPath.c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Load UI Layout")) {
        std::string projRoot = Application::Get().GetProjectRoot();
        if (projRoot.empty()) projRoot = ".";
        std::string layoutPath = (std::filesystem::path(projRoot) / "ui_layout.json").string();
        UICreationEngine::LoadLayout(layoutPath);
    }
}






void EditorLayer::DrawBuildModal(Scene& scene) {
    if (!m_ShowBuildModal) return;

    ImGuiIO& io = ImGui::GetIO();
    float winW = 520.0f, winH = 460.0f;
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - winW) * 0.5f,
                                   (io.DisplaySize.y - winH) * 0.5f),
                            ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Appearing);
    ImGui::SetNextWindowBgAlpha(0.97f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    bool open = true;
    if (!ImGui::Begin("  Build Window", &open, flags)) {
        ImGui::End();
        if (!open) m_ShowBuildModal = false;
        return;
    }
    if (!open) { m_ShowBuildModal = false; ImGui::End(); return; }

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.85f, 1.0f, 1.0f));
    ImGui::Text(" Calcium3D — Build System");
    ImGui::PopStyleColor();
    ImGui::SameLine(winW - 180);
    ImGui::TextDisabled("Project: %s", Application::Get().GetProjectName().c_str());
    ImGui::Separator();

    bool building = m_BuildInProgress;
    if (building) ImGui::BeginDisabled();

    
    ImGui::Spacing();
    ImGui::Text("Target Platform");
    ImGui::Spacing();
    const char* platforms[] = { "Linux x86_64", "Windows x64", "Android" };
    const ImVec4 platformColors[3] = {
        ImVec4(0.2f, 0.75f, 0.35f, 1.0f),   
        ImVec4(0.3f, 0.55f, 0.9f,  1.0f),   
        ImVec4(0.85f, 0.55f, 0.1f, 1.0f),   
    };
    for (int i = 0; i < 3; i++) {
        bool selected = (m_BuildPlatform == i);
        if (selected) {
            ImGui::PushStyleColor(ImGuiCol_Button,        platformColors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, platformColors[i]);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  platformColors[i]);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.32f, 0.32f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        }
        if (ImGui::Button(platforms[i], ImVec2(155, 34))) m_BuildPlatform = i;
        ImGui::PopStyleColor(3);
        if (i < 2) ImGui::SameLine();
    }

    
    if (m_BuildPlatform == 1 || m_BuildPlatform == 2) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.15f, 1.0f));
        ImGui::TextWrapped("  This build of the engine does not support %s yet."
                           " Only Linux x86_64 is available in this release.",
                           platforms[m_BuildPlatform]);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();

    
    bool linuxSelected = (m_BuildPlatform == 0);
    if (!linuxSelected) ImGui::BeginDisabled();

    ImGui::Spacing();
    ImGui::Text("Output Path");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##outpath", m_BuildOutputPath, sizeof(m_BuildOutputPath));

    ImGui::Spacing();
    ImGui::Text("Start Scene");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##startscene", m_BuildStartScene, sizeof(m_BuildStartScene));

    ImGui::Spacing();
    ImGui::Text("Start State");
    auto& allStates = GameStateManager::GetAllStates();
    std::vector<const char*> stateNames;
    std::vector<int> stateIds;
    for (auto const& [id, name] : allStates) {
        stateNames.push_back(name.c_str());
        stateIds.push_back(id);
    }
    int currentStart = (int)Application::Get().m_StartGameState;
    int startIndex = 0;
    for (int i = 0; i < (int)stateIds.size(); i++)
        if (stateIds[i] == currentStart) startIndex = i;
    ImGui::SetNextItemWidth(-1);
    if (!stateNames.empty())
        if (ImGui::Combo("##startstate", &startIndex, stateNames.data(), (int)stateNames.size()))
            Application::Get().m_StartGameState = (GameState)stateIds[startIndex];

    ImGui::Spacing();
    ImGui::Checkbox("Disable state warning in build", &m_DisableStateWarning);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("When enabled, the fallback warning screen will not\nappear in the standalone build even if states are misconfigured.");

    if (!linuxSelected) ImGui::EndDisabled();

    if (building) ImGui::EndDisabled();

    
    ImGui::Spacing();
    float progress = m_BuildProgress.load();
    std::string stageMsg;
    {
        std::lock_guard<std::mutex> lock(m_BuildMsgMutex);
        stageMsg = m_BuildStageMsg;
    }
    if (m_BuildInProgress || m_BuildDone) {
        ImVec4 barColor;
        if (m_BuildDone)
            barColor = m_BuildSuccess ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f)
                                      : ImVec4(0.85f, 0.2f, 0.2f, 1.0f);
        else
            barColor = ImVec4(0.2f, 0.55f, 1.0f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
        char overlay[64];
        snprintf(overlay, sizeof(overlay),
                 m_BuildDone ? (m_BuildSuccess ? "Done!" : "Failed") : "Building... %.0f%%",
                 progress * 100.0f);
        ImGui::ProgressBar(progress, ImVec2(-1, 22), overlay);
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 0.75f, 1.0f));
        ImGui::TextWrapped("%s", stageMsg.c_str());
        ImGui::PopStyleColor();
    }

    
    ImGui::Spacing();
    float buttonY = winH - 55.0f;
    ImGui::SetCursorPosY(buttonY);
    ImGui::Separator();
    ImGui::Spacing();

    bool canBuild = !m_BuildInProgress && linuxSelected && !Application::Get().GetProjectRoot().empty();

    if (!canBuild) ImGui::BeginDisabled();
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.5f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f,  0.4f, 0.8f, 1.0f));
    bool clicked = ImGui::Button("  Build  ", ImVec2(120, 34));
    ImGui::PopStyleColor(3);
    if (!canBuild) ImGui::EndDisabled();

    if (clicked && linuxSelected) {
        std::string projRoot   = Application::Get().GetProjectRoot();
        std::string outputPath = m_BuildOutputPath;
        int startState         = (int)Application::Get().m_StartGameState;
        auto customStates      = GameStateManager::GetAllStates();

        auto* edApp = dynamic_cast<EditorApplication*>(&Application::Get());
        if (edApp && edApp->IsPlayMode()) edApp->ExitPlayMode();

        std::filesystem::path scenePath = Application::Get().GetScene()->GetFilepath();
        if (scenePath.empty() || scenePath.filename() == "calcium3d_playmode_backup.scene")
            scenePath = std::filesystem::path(projRoot) / "Scenes" / "main.scene";
        Application::Get().GetScene()->Save(scenePath.string());
        UICreationEngine::SaveLayout((std::filesystem::path(projRoot) / "ui_layout.json").string());

        nlohmann::json envSetup;
        envSetup["showSkybox"]         = showSkybox;
        envSetup["showGradientSky"]    = showGradientSky;
        envSetup["showWater"]          = showWater;
        envSetup["showClouds"]         = showClouds;
        envSetup["cloudMode"]          = cloudMode;
        envSetup["waterHeight"]        = waterHeight;
        envSetup["cloudHeight"]        = cloud2dHeight;
        envSetup["cloudDensity"]       = cloudDensity;
        envSetup["cloudCover"]         = cloudCover;
        envSetup["waveSpeed"]          = waveSpeed;
        envSetup["waveStrength"]       = waveStrength;
        envSetup["waterColor"]         = {waterColor.x, waterColor.y, waterColor.z};
        envSetup["timeOfDay"]          = timeOfDay;
        envSetup["sunEnabled"]         = sunEnabled;
        envSetup["sunIntensity"]       = sunIntensity;
        envSetup["sunColor"]           = {sunColor.r, sunColor.g, sunColor.b, sunColor.a};
        envSetup["sunBloom"]           = sunBloom;
        envSetup["moonEnabled"]        = moonEnabled;
        envSetup["moonIntensity"]      = moonIntensity;
        envSetup["moonColor"]          = {moonColor.r, moonColor.g, moonColor.b, moonColor.a};
        envSetup["moonBloom"]          = moonBloom;
        envSetup["globalTilingFactor"] = globalTilingFactor;
        Camera* cam = Application::Get().GetCamera();
        if (cam) {
            envSetup["camera"]["position"]    = nlohmann::json::array({cam->Position.x, cam->Position.y, cam->Position.z});
            envSetup["camera"]["orientation"] = nlohmann::json::array({cam->Orientation.x, cam->Orientation.y, cam->Orientation.z});
            envSetup["camera"]["yaw"]   = cam->yaw;
            envSetup["camera"]["pitch"] = cam->pitch;
            envSetup["camera"]["fov"]   = cam->FOV;
        }

        BuildManager::BuildSettings settings;
        settings.ProjectRoot         = projRoot;
        settings.StartScene          = scenePath.filename().string();
        settings.StartGameState      = startState;
        settings.CustomGameStates    = customStates;
        settings.EnvironmentSettings = envSetup;
        settings.DisableStateWarning = m_DisableStateWarning;
        settings.OutputPath = (std::filesystem::path(outputPath).is_absolute()
            ? std::filesystem::path(outputPath)
            : std::filesystem::path(projRoot) / outputPath).string();

        m_BuildInProgress = true;
        m_BuildDone       = false;
        m_BuildSuccess    = false;
        m_BuildProgress   = 0.0f;
        { std::lock_guard<std::mutex> lock(m_BuildMsgMutex); m_BuildStageMsg = "Preparing build..."; }

        if (m_BuildThread.joinable()) m_BuildThread.join();
        m_BuildThread = std::thread([this, settings]() mutable {
            auto setMsg = [this](const std::string& msg) {
                std::lock_guard<std::mutex> lock(m_BuildMsgMutex);
                m_BuildStageMsg = msg;
                Logger::AddLog("[Build] %s", msg.c_str());
            };
            setMsg("Creating directory structure...");   m_BuildProgress = 0.05f;
            setMsg("Copying project data...");            m_BuildProgress = 0.20f;
            setMsg("Copying engine internals...");        m_BuildProgress = 0.35f;
            setMsg("Writing meson.build...");             m_BuildProgress = 0.45f;
            setMsg("Running meson setup...");             m_BuildProgress = 0.55f;
            setMsg("Compiling — this may take a minute..."); m_BuildProgress = 0.65f;
            bool ok = BuildManager::Build(settings);
            m_BuildProgress   = 1.0f;
            m_BuildSuccess    = ok;
            m_BuildDone       = true;
            m_BuildInProgress = false;
            setMsg(ok ? "\xe2\x9c\x93 Build succeeded! Output: " + settings.OutputPath
                      : "\xe2\x9c\x97 Build failed \xe2\x80\x94 check Console for details.");
        });
    }

    ImGui::SameLine();
    if (m_BuildInProgress) ImGui::BeginDisabled();
    if (ImGui::Button("Close", ImVec2(80, 34))) {
        if (m_BuildThread.joinable()) m_BuildThread.join();
        m_ShowBuildModal = false;
    }
    if (m_BuildInProgress) ImGui::EndDisabled();

    ImGui::End();
}

