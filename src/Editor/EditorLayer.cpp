#include "EditorLayer.h"
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
#include <imgui_internal.h> 
#include <filesystem>
#include <imgui.h>
#include "UI/UIManager.h"
#include "UI/UICreationEngine.h"
#include "Core/GameState.h"

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

    
    ImGuiID dock_left, dock_remaining;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.18f, &dock_left, &dock_remaining);

    
    ImGuiID dock_right;
    ImGui::DockBuilderSplitNode(dock_remaining, ImGuiDir_Right, 0.25f, &dock_right, &dock_remaining);

    
    ImGuiID dock_bottom;
    ImGui::DockBuilderSplitNode(dock_remaining, ImGuiDir_Down, 0.25f, &dock_bottom, &dock_remaining);

    

    
    ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left);
    ImGui::DockBuilderDockWindow("Inspector", dock_left);
    ImGui::DockBuilderDockWindow("Settings", dock_right);
    ImGui::DockBuilderDockWindow("Viewport", dock_remaining);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);
    ImGui::DockBuilderDockWindow("Content Browser", dock_bottom);

    ImGui::DockBuilderFinish(dockspace_id);
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
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                
                
                selectedCube = -1;
                selectedMesh = -1;
                isLightSelected = false;
                isLightSelected = false;
                selectedPointLightIndex = -1;
                Logger::AddLog("Cleared all selections");
            }
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(m_Window, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("Build Settings...")) {
                ImGui::OpenPopup("Build Settings");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Build (Linux)")) {
                BuildManager::BuildSettings settings;
                settings.ProjectRoot = Application::Get().GetProjectRoot();
                settings.OutputPath = (std::filesystem::path(settings.ProjectRoot) / "Build" / "Linux").string();
                BuildManager::Build(settings);
            }
            ImGui::EndMenu();
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
                    
                    scene.AddObject(GameObject(std::move(cubeMesh), "Cube"));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Cube");
                }
                if (ImGui::MenuItem("Sphere")) {
                    Mesh sphereMesh = ObjectFactory::createSphere(32, 16);
                    
                    GameObject obj(std::move(sphereMesh), "Sphere");
                    obj.shape = ColliderShape::Sphere;
                    scene.AddObject(std::move(obj));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Sphere");
                }
                if (ImGui::MenuItem("Plane")) {
                    Mesh planeMesh = ObjectFactory::createPlane();
                    
                    scene.AddObject(GameObject(std::move(planeMesh), "Plane"));
                    selectedCube = (int)scene.GetObjects().size() - 1;
                    isLightSelected = false; selectedPointLightIndex = -1; selectedMesh = -1;
                    Logger::AddLog("Created Plane");
                }
                ImGui::EndMenu();
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
                 ImGui::MenuItem("Camera", NULL, false, false); 
                 ImGui::EndMenu();
             }
             
            ImGui::EndMenu();
        }
        
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 300);
        float fps = ImGui::GetIO().Framerate;
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "%.1f FPS (%.2f ms)", fps, 1000.0f / fps);
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Mode: %s", Editor::isEditMode ? "EDITOR" : "PLAY");

        ImGui::EndMainMenuBar();
    }
}

void EditorLayer::DrawSceneHierarchy(Scene& scene) {
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Scene Hierarchy", nullptr, lockFlags);
    
    if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& objects = scene.GetObjects();
        for (int i = 0; i < objects.size(); ++i) {
            std::string name = objects[i].name ;
            if (name.empty()) name = "GameObject " + std::to_string(i);
            
            
            name += "##" + std::to_string(i);
            
            bool isSelected = (selectedCube == i);
            if (ImGui::Selectable(name.c_str(), isSelected)) {
                selectedCube = i;
                selectedMesh = -1;
                isLightSelected = false;
                selectedPointLightIndex = -1;
                Logger::AddLog("Selected %s", objects[i].name.c_str());
            }
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

        ImGui::Separator();
        ImGui::Text("Load Hierarchy (Start State)");
        const char* stateNames[] = { "0: Start Screen", "1: Gameplay", "2: Exit", "3: Pause", "4: Settings" };
        int currentStart = (int)Application::Get().m_StartGameState;
        if (ImGui::Combo("Start State", &currentStart, stateNames, IM_ARRAYSIZE(stateNames))) {
            Application::Get().m_StartGameState = (GameState)currentStart;
        }
        
        ImGui::Separator();
        ImGui::Text("Info");
        ImGui::Text("Calcium3D Engine v0.1");
        ImGui::Text("OpenGL %s", glGetString(GL_VERSION));
        ImGui::Text("GPU: %s", glGetString(GL_RENDERER));
    }
    
    if (ImGui::CollapsingHeader("UI Editor")) {
        DrawUIEditor();
    }
    
    ImGui::Separator();
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void EditorLayer::DrawViewport(Scene& scene, Camera& camera) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags lockFlags = fixedLayout ? 
        (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse) : 0;
    
    ImGui::Begin("Viewport", nullptr, lockFlags);
    
    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    
    
    viewportWidth = (int)viewportPanelSize.x;
    viewportHeight = (int)viewportPanelSize.y;
    
    if (viewportTextureID != 0 && viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
        
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImGui::Image(
            (ImTextureID)(intptr_t)viewportTextureID,
            viewportPanelSize,
            ImVec2(0, 1),  
            ImVec2(1, 0)   
        );
        
        
        if ((ImGui::IsWindowFocused() || ImGui::IsWindowHovered()) && 
            !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            if (ImGui::IsKeyPressed(ImGuiKey_W)) gizmoOp = 0; 
            if (ImGui::IsKeyPressed(ImGuiKey_E)) gizmoOp = 1; 
            if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOp = 2; 
        }
        
        
        if (uiEditMode) {
            auto& elements = UICreationEngine::GetElements();
            for (int i = 0; i < (int)elements.size(); i++) {
                auto& el = elements[i];
                
                
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

        
        
        
        
        
        
        UIManager::Render(UICreationEngine::GetElements(), glm::vec2(viewportWidth, viewportHeight), glm::vec2(cursorPos.x, cursorPos.y));
        
        bool gizmoUsing = ImGuizmo::IsUsing();
        bool gizmoOver = ImGuizmo::IsOver();
        
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) 
            && !gizmoUsing && !gizmoOver) {
            
            ImVec2 mousePos = ImGui::GetMousePos();
            float mx = mousePos.x - cursorPos.x;
            float my = mousePos.y - cursorPos.y;
            
            
            float ndcX = (2.0f * mx) / viewportPanelSize.x - 1.0f;
            float ndcY = 1.0f - (2.0f * my) / viewportPanelSize.y; 
            
            
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
                     float screenX = (ndc.x + 1.0f) * 0.5f * viewportPanelSize.x;
                     float screenY = (1.0f - ndc.y) * 0.5f * viewportPanelSize.y;
                     
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
                     float screenX = cursorPos.x + (ndc.x + 1.0f) * 0.5f * viewportPanelSize.x;
                     float screenY = cursorPos.y + (1.0f - ndc.y) * 0.5f * viewportPanelSize.y;
                     
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
                
                
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix, obj.position);
                modelMatrix *= glm::mat4_cast(obj.rotation);
                modelMatrix = glm::scale(modelMatrix, obj.scale);
                
                
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportPanelSize.x, viewportPanelSize.y);
                
                
                ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
                if (gizmoOp == 1) op = ImGuizmo::ROTATE;
                if (gizmoOp == 2) op = ImGuizmo::SCALE;
                ImGuizmo::MODE mode = gizmoLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
                
                
                if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), op, mode, glm::value_ptr(modelMatrix))) {
                    glm::vec3 skew; glm::vec4 perspective; glm::vec3 newPos, newScale; glm::quat newRot;
                    glm::decompose(modelMatrix, newScale, newRot, newPos, skew, perspective);
                    obj.position = newPos;
                    obj.rotation = newRot;
                    obj.scale = newScale;
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
                 ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportPanelSize.x, viewportPanelSize.y);
                 
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
    ImGui::SameLine();
    ImGui::Text("> %s", m_CurrentContentPath.c_str());
    ImGui::Separator();

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
    
    ImGui::Columns(columns, nullptr, false);
    
    try {
        for (auto& entry : fs::directory_iterator(m_CurrentContentPath)) {
            std::string filename = entry.path().filename().string();
            
            ImGui::PushID(filename.c_str());
            
            
            ImVec4 iconColor = entry.is_directory() ? ImVec4(1.0f, 0.9f, 0.5f, 1.0f) : ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, iconColor);
            
            if (ImGui::Button("##Item", ImVec2(cellSize, cellSize))) {
                
            }
            
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (entry.is_directory()) {
                    m_CurrentContentPath = entry.path().string();
                }
            }

            ImGui::PopStyleColor();
            
            ImGui::TextWrapped("%s", filename.c_str());
            
            ImGui::NextColumn();
            ImGui::PopID();
        }
    } catch (...) {}
    
    ImGui::Columns(1);
}
void EditorLayer::DrawUIEditor() {
    static char filter[128] = "";
    ImGui::Checkbox("UI Edit Mode", &uiEditMode);
    ImGui::Separator();

    if (ImGui::Button("Add Button")) {
        UIElement btn;
        btn.name = "Button_" + std::to_string(UICreationEngine::GetElements().size());
        btn.type = UIElementType::BUTTON;
        btn.text = "Click Me";
        btn.screenName = (strlen(filter) > 0) ? filter : "StartScreen";
        UICreationEngine::AddElement(btn);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Text")) {
        UIElement txt;
        txt.name = "Text_" + std::to_string(UICreationEngine::GetElements().size());
        txt.type = UIElementType::TEXT;
        txt.text = "Hello UI";
        txt.screenName = (strlen(filter) > 0) ? filter : "StartScreen";
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

        const char* screenOptions[] = { "StartScreen", "GameScreen", "HUD", "PauseMenu" };
        int currentScreenIdx = 0;
        for (int i = 0; i < 4; i++) {
            if (el.screenName == screenOptions[i]) currentScreenIdx = i;
        }
        if (ImGui::Combo("Screen", &currentScreenIdx, screenOptions, 4)) {
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
        
        if (ImGui::Button("Delete Element")) {
            elements.erase(elements.begin() + selectedUIElement);
            selectedUIElement = -1;
        }
    }

    ImGui::Separator();
    if (ImGui::Button("Save UI Layout")) {
        UICreationEngine::SaveLayout("ui_layout.json");
    }
    ImGui::SameLine();
    if (ImGui::Button("Load UI Layout")) {
        UICreationEngine::LoadLayout("ui_layout.json");
    }
}
