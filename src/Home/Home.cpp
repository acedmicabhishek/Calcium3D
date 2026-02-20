#include "Home.h"
#include <imgui.h>
#include "../Core/Application.h"
#include "../Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

Home::Home() {
    const char* homeDir = getenv("HOME");
    if (homeDir) {
        m_BaseProjectsPath = fs::path(homeDir) / "Documents" / "Calcium3DProjects";
    } else {
        m_BaseProjectsPath = fs::current_path().parent_path() / "Calcium3DProjects";
    }

    if (!fs::exists(m_BaseProjectsPath)) {
        fs::create_directories(m_BaseProjectsPath);
    }

    LoadRecentProjects();
}

Home::~Home() {
}

void Home::Init(GLFWwindow* window) {
    m_Window = window;
}

void Home::Render() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("Project Manager", nullptr, window_flags)) {
        
        ImGui::SetCursorPos(ImVec2(viewport->WorkSize.x * 0.5f - 200, viewport->WorkSize.y * 0.15f));
        ImGui::SetWindowFontScale(2.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Calcium3D");
        ImGui::SetWindowFontScale(1.0f);
        
        ImGui::SetCursorPosX(viewport->WorkSize.x * 0.5f - 180);
        ImGui::Text("The Lightweight High-Performance 3D Engine");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        
        float buttonWidth = 250.0f;
        float centerX = viewport->WorkSize.x * 0.5f - buttonWidth * 0.5f;
        
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("New Project", ImVec2(buttonWidth, 40))) {
            
            ImGui::OpenPopup("New Project");
        }
        
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Open Project", ImVec2(buttonWidth, 40))) {
            
            
            Application::Get().OpenProject(m_BaseProjectsPath.string());
        }

        
        if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char projectName[128] = "MyNewProject";
            ImGui::InputText("Project Name", projectName, IM_ARRAYSIZE(projectName));
            
            if (ImGui::Button("Create", ImVec2(120, 0))) {
                fs::path projectPath = m_BaseProjectsPath / std::string(projectName);
                Application::Get().CreateProject(projectPath.string());
                SaveRecentProject(projectPath.string());
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::SetCursorPosY(viewport->WorkSize.y * 0.5f);
        ImGui::SetCursorPosX(centerX);
        ImGui::Text("Recent Projects:");
        
        ImGui::BeginChild("RecentProjects", ImVec2(buttonWidth, 200), true);
        for (const auto& path : m_RecentProjects) {
            if (ImGui::Selectable(path.c_str())) {
                Application::Get().OpenProject(path);
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void Home::LoadRecentProjects() {
    
    fs::path engineResource = fs::current_path().parent_path() / "Resource";
    if (fs::exists(engineResource)) {
        m_RecentProjects.push_back(engineResource.string());
    }
    
    
    try {
        if (fs::exists(m_BaseProjectsPath)) {
            for (const auto& entry : fs::directory_iterator(m_BaseProjectsPath)) {
                if (entry.is_directory()) {
                    m_RecentProjects.push_back(entry.path().string());
                }
            }
        }
    } catch (...) {}
    
    
}

void Home::SaveRecentProject(const std::string& path) {
    for (const auto& p : m_RecentProjects) {
        if (p == path) return;
    }
    m_RecentProjects.push_back(path);
}
