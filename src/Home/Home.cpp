#include "Home.h"
#include <imgui.h>
#include "../Core/Application.h"
#include "../Core/Logger.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include "../Core/DependencyManager.h"
#include <map>
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
        if (ImGui::Button("Play Demos", ImVec2(buttonWidth, 40))) {
            ImGui::OpenPopup("Play Demos");
        }
        
        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Open Project", ImVec2(buttonWidth, 40))) {
            char buffer[256];
            std::string selectedPath = "";
            FILE* pipe = popen("zenity --file-selection --directory --title=\"Select Calcium3D Project Folder\"", "r");
            if (pipe) {
                while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                    selectedPath += buffer;
                }
                pclose(pipe);
                if (!selectedPath.empty() && selectedPath.back() == '\n') {
                    selectedPath.pop_back(); 
                }
                if (!selectedPath.empty()) {
                    Application::Get().OpenProject(selectedPath);
                }
            } else {
                Logger::AddLog("[ERROR] Failed to open native file dialog (Zenity required).");
                Application::Get().OpenProject(m_BaseProjectsPath.string());
            }
        }

        ImGui::SetCursorPosX(centerX);
        if (ImGui::Button("Install Dependencies", ImVec2(buttonWidth, 40))) {
            ImGui::OpenPopup("Dependency Manager");
        }

        if (ImGui::BeginPopupModal("Dependency Manager", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("System Requirements:");
            ImGui::Separator();
            
            static std::vector<DependencyInfo> s_Deps = DependencyManager::GetDependencies();
            static float lastRefresh = 0;
            if (ImGui::GetTime() - lastRefresh > 5.0f) {
                s_Deps = DependencyManager::GetDependencies();
                lastRefresh = ImGui::GetTime();
            }

            bool allInstalled = true;
            
            ImGui::BeginChild("DepList", ImVec2(500, 300), true);
            for (const auto& dep : s_Deps) {
                ImGui::PushID(dep.packageName.c_str());
                
                if (dep.isInstalled) {
                    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "[INSTALLED]");
                } else {
                    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[MISSING  ]");
                    allInstalled = false;
                }
                
                ImGui::SameLine(100);
                ImGui::Text("%s", dep.name.c_str());
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (%s)", dep.description.c_str(), dep.packageName.c_str());
                
                ImGui::SameLine(300);
                if (dep.isInstalled) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button("Uninstall", ImVec2(80, 0))) {
                        DependencyManager::UninstallPackage(dep);
                        s_Deps = DependencyManager::GetDependencies();
                    }
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                    if (ImGui::Button("Install", ImVec2(80, 0))) {
                        DependencyManager::InstallPackage(dep);
                        s_Deps = DependencyManager::GetDependencies();
                    }
                    ImGui::PopStyleColor();
                }
                
                ImGui::PopID();
                ImGui::Separator();
            }
            ImGui::EndChild();
            
            ImGui::Separator();
            
            std::string pm = DependencyManager::GetPackageManager();
            ImGui::Text("Detected Package Manager: %s", pm.empty() ? "None (Manual Install required)" : pm.c_str());

            if (!pm.empty()) {
                if (ImGui::Button("Install All Missing", ImVec2(200, 40))) {
                    DependencyManager::InstallAll();
                    s_Deps = DependencyManager::GetDependencies();
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Warning: No supported package manager found.");
                ImGui::Text("Please install dependencies manually using your distro's tools.");
            }
            
            ImGui::Spacing();
            if (ImGui::Button("Refresh", ImVec2(120, 0))) {
                s_Deps = DependencyManager::GetDependencies();
            }
            ImGui::SameLine();
            if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
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
        
        if (ImGui::BeginPopupModal("Play Demos", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static int selectedDemoDir = 0;
            static std::vector<std::string> demoPaths;
            static std::vector<std::string> demoNames;
            
            if (demoPaths.empty()) {
                fs::path demosPath = fs::current_path().parent_path() / "Resource" / "Demos";
                if (fs::exists(demosPath)) {
                    for (const auto& entry : fs::directory_iterator(demosPath)) {
                        if (entry.is_directory()) {
                            demoPaths.push_back(entry.path().string());
                            demoNames.push_back(entry.path().filename().string());
                        }
                    }
                }
            }
            
            ImGui::Text("Select a Demo Template:");
            if (demoNames.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No Demos found in Resource/Demos/");
            } else {
                if (ImGui::BeginCombo("##democombo", demoNames[selectedDemoDir].c_str())) {
                    for (int n = 0; n < demoNames.size(); n++) {
                        bool is_selected = (selectedDemoDir == n);
                        if (ImGui::Selectable(demoNames[n].c_str(), is_selected)) {
                            selectedDemoDir = n;
                        }
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            
            ImGui::Spacing();
            static char demoProjectName[128] = "MyDemoProject";
            ImGui::InputText("Save As", demoProjectName, IM_ARRAYSIZE(demoProjectName));
            
            static bool demoExistsError = false;
            if (demoExistsError) {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Name already taken! Change 'Save As' name.");
            }

            if (ImGui::Button("Load Demo", ImVec2(120, 0)) && !demoPaths.empty()) {
                fs::path destPath = m_BaseProjectsPath / std::string(demoProjectName);
                if (!fs::exists(destPath)) {
                    demoExistsError = false;
                    fs::copy(demoPaths[selectedDemoDir], destPath, fs::copy_options::recursive);
                    SaveRecentProject(destPath.string());
                    Application::Get().OpenProject(destPath.string());
                    ImGui::CloseCurrentPopup();
                } else {
                    demoExistsError = true;
                    Logger::AddLog("[ERROR] Project folder already exists. Choose a different save name.");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        ImGui::SetCursorPosY(viewport->WorkSize.y * 0.45f);
        ImGui::SetCursorPosX(centerX);
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::Text("Recent Projects:");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        ImGui::SetCursorPosX(centerX);
        
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 0.8f));
        
        ImGui::BeginChild("RecentProjects", ImVec2(buttonWidth, 220), true);
        
        for (const auto& path : m_RecentProjects) {
            std::string dirName = std::filesystem::path(path).filename().string();
            
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            if (ImGui::Selectable(dirName.c_str())) {
                Application::Get().OpenProject(path);
            }
            ImGui::PopStyleColor();
            
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", path.c_str());
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
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
