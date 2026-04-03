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
#include <filesystem>
#include <glad/glad.h>
#include <imgui.h>

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

  strncpy(m_ProjectPath, m_BaseProjectsPath.string().c_str(), sizeof(m_ProjectPath));
}

Home::~Home() {
  for (auto const &[name, texture] : m_DemoPreviews) {
    texture->Delete();
    delete texture;
  }
  for (auto const &[path, texture] : m_ProjectThumbnails) {
    texture->Delete();
    delete texture;
  }
}

void Home::Init(GLFWwindow *window) {
  m_Window = window;
  LoadDemoPreviews();
}

void Home::Update(float deltaTime) {
  if (m_IsCreatingProject) {
    m_CreationProgress += deltaTime * 0.4f; 
    
    if (m_CreationProgress < 0.3f) m_CreationStatus = "Initializing project structure...";
    else if (m_CreationProgress < 0.6f) m_CreationStatus = "Copying template assets...";
    else if (m_CreationProgress < 0.9f) m_CreationStatus = "Configuring project metadata...";
    else m_CreationStatus = "Opening Calcium3D Editor...";

    if (m_CreationProgress >= 1.0f) {
      m_IsCreatingProject = false;
      m_CreationProgress = 0.0f;
      
      fs::path finalPath = fs::path(m_ProjectPath) / m_ProjectName;
      if (!fs::exists(finalPath)) {
          fs::create_directories(finalPath);
          SaveRecentProject(finalPath.string());
          Application::Get().OpenProject(finalPath.string());
      }
    }
  }

  if (m_IsLoadingProject) {
    m_CreationProgress += deltaTime * 0.8f; 
    
    if (m_CreationProgress < 0.3f) m_CreationStatus = "Loading scene data...";
    else if (m_CreationProgress < 0.6f) m_CreationStatus = "Optimizing shaders...";
    else if (m_CreationProgress < 0.9f) m_CreationStatus = "Rebuilding buffers...";
    else m_CreationStatus = "Opening Calcium3D Editor...";

    if (m_CreationProgress >= 1.0f) {
      m_IsLoadingProject = false;
      m_CreationProgress = 0.0f;
      Application::Get().OpenProject(m_LoadingProjectPath);
    }
  }
}

void Home::Render() {
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  if (ImGui::Begin("Home", nullptr, window_flags)) {
    float sidebarWidth = 220.0f;
    float bottomBarHeight = 100.0f;
    float rightPanelWidth = (m_CurrentTab == HomeTab::Demos) ? 400.0f : 0.0f;

    RenderSidebar(sidebarWidth, viewport->WorkSize.y - bottomBarHeight);

    ImGui::SameLine();

    
    ImGui::BeginGroup();
    {
      float mainContentWidth = viewport->WorkSize.x - sidebarWidth - rightPanelWidth;
      float mainContentHeight = viewport->WorkSize.y - bottomBarHeight;

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
      ImGui::BeginChild("MainView", ImVec2(mainContentWidth, mainContentHeight), 
                        false, ImGuiWindowFlags_NoBackground);

      switch (m_CurrentTab) {
      case HomeTab::Projects:
        RenderProjectsView();
        break;
      case HomeTab::Demos:
        RenderDemosView();
        break;
      case HomeTab::Settings:
        RenderSettingsView();
        break;
      }

      ImGui::EndChild();
      ImGui::PopStyleVar();

      if (rightPanelWidth > 0) {
        ImGui::SameLine();
        RenderDemoDetails(rightPanelWidth, mainContentHeight);
      }
    }
    ImGui::EndGroup();

    
    if (m_CurrentTab == HomeTab::Demos) {
        ImGui::SetCursorPos(ImVec2(sidebarWidth, viewport->WorkSize.y - bottomBarHeight));
        RenderCreationBar(viewport->WorkSize.x - sidebarWidth, bottomBarHeight);
    }
    
    
    if (m_IsCreatingProject || m_IsLoadingProject) {
        RenderLoadingOverlay();
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();

  
  if (m_ShowDeletePopup) {
      ImGui::OpenPopup("Delete Project?");
      m_ShowDeletePopup = false;
  }

  if (ImGui::BeginPopupModal("Delete Project?", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Are you sure you want to permanently delete this "
                "project?\nThis action cannot be undone.\n\nPath: %s",
                m_ProjectToDelete.c_str());
    ImGui::Separator();

    if (ImGui::Button("Delete", ImVec2(120, 0))) {
      DeleteProject(m_ProjectToDelete);
      m_ProjectToDelete = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      m_ProjectToDelete = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void Home::RenderSidebar(float width, float height) {
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.07f, 0.07f, 0.09f, 1.0f));
  ImGui::BeginChild("Sidebar", ImVec2(width, height), false, ImGuiWindowFlags_NoScrollbar);

  
  ImGui::Spacing(); ImGui::Spacing();
  ImGui::Indent(20);
  ImGui::SetWindowFontScale(1.4f);
  ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Calcium3D");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.8f), "Engine v1.0");
  ImGui::Unindent(20);
  ImGui::Spacing(); ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing(); ImGui::Spacing();

  
  struct TabItem { const char* label; HomeTab tab; };
  TabItem tabs[] = {
      { "Recent Projects", HomeTab::Projects },
      { "Demo Scenes",     HomeTab::Demos },
      { "Settings",        HomeTab::Settings },
  };

  for (auto& item : tabs) {
      bool active = (m_CurrentTab == item.tab);
      ImGui::PushID(item.label);

      
      ImVec2 pos = ImGui::GetCursorScreenPos();
      float rowHeight = 42.0f;

      
      if (active) {
          ImGui::GetWindowDrawList()->AddRectFilled(
              pos, ImVec2(pos.x + width, pos.y + rowHeight),
              ImColor(0.35f, 0.65f, 1.0f, 0.12f));
          
          ImGui::GetWindowDrawList()->AddRectFilled(
              pos, ImVec2(pos.x + 3, pos.y + rowHeight),
              ImColor(0.35f, 0.65f, 1.0f, 1.0f));
      }

      ImGui::SetCursorPosX(20);
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.05f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.08f));

      char btnLabel[128];
      snprintf(btnLabel, sizeof(btnLabel), "  %s", item.label);
      if (ImGui::Button(btnLabel, ImVec2(width - 20, rowHeight))) {
          m_CurrentTab = item.tab;
      }
      ImGui::PopStyleColor(3);
      ImGui::Spacing();
      ImGui::PopID();
  }

  
  float bottomY = height - 60;
  ImGui::SetCursorPos(ImVec2(20, bottomY));
  ImGui::Separator();
  ImGui::Spacing();
  ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 0.6f), "Calcium3D Engine");
  ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 0.4f), "Build 2026.03 | OpenGL 4.6");

  ImGui::EndChild();
  ImGui::PopStyleColor();
}

void Home::RenderProjectsView() {
  ImGui::SetWindowFontScale(1.1f);
  ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Recent Projects");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::Spacing();

  float cardWidth = 220.0f;
  float cardHeight = 160.0f;
  float cardSpacing = 15.0f;
  float totalCardWidth = cardWidth + cardSpacing;
  int columns = (int)(ImGui::GetContentRegionAvail().x / totalCardWidth);
  if (columns < 1) columns = 1;

  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(cardSpacing / 2, cardSpacing / 2));
  if (ImGui::BeginTable("ProjectsGrid", columns)) {

    
    ImGui::TableNextColumn();
    ImGui::PushID("NewProjectCard");
    {
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        Texture *newProjTex = m_DemoPreviews["new_project"];

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        bool clicked = false;
        if (newProjTex) {
            clicked = ImGui::ImageButton("##NewBtn", (ImTextureID)newProjTex->ID, ImVec2(cardWidth, cardHeight), ImVec2(0, 1), ImVec2(1, 0));
        } else {
            clicked = ImGui::Button("+\nNew Project", ImVec2(cardWidth, cardHeight));
        }
        if (clicked) {
            m_ShowNewProjectModal = true;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "New Project");
    }
    ImGui::PopID();

    
    ImGui::TableNextColumn();
    ImGui::PushID("OpenProjectCard");
    {
        Texture *openProjTex = m_DemoPreviews["open_project"];

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        bool clicked = false;
        if (openProjTex) {
            clicked = ImGui::ImageButton("##OpenBtn", (ImTextureID)openProjTex->ID, ImVec2(cardWidth, cardHeight), ImVec2(0, 1), ImVec2(1, 0));
        } else {
            clicked = ImGui::Button("Browse\nOpen Project", ImVec2(cardWidth, cardHeight));
        }
        if (clicked) {
            char buffer[256];
            std::string selectedPath = "";
            FILE *pipe = popen("zenity --file-selection --directory --title=\"Select Project Folder\"", "r");
            if (pipe) {
                while (fgets(buffer, sizeof(buffer), pipe) != NULL) selectedPath += buffer;
                pclose(pipe);
                if (!selectedPath.empty() && selectedPath.back() == '\n') selectedPath.pop_back();
                if (!selectedPath.empty()) Application::Get().OpenProject(selectedPath);
            }
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Open Project");
    }
    ImGui::PopID();

    
    for (int i = 0; i < (int)m_RecentProjects.size(); i++) {
        ImGui::TableNextColumn();
        ImGui::PushID(i + 100);

        std::string projectPath = m_RecentProjects[i];
        std::string projectName = fs::path(projectPath).filename().string();

        
        Texture* thumb = nullptr;
        if (m_ProjectThumbnails.count(projectPath)) {
            thumb = m_ProjectThumbnails[projectPath];
        } else {
            std::string thumbFile = projectPath + "/thumbnail.png";
            if (fs::exists(thumbFile)) {
                thumb = new Texture(thumbFile.c_str());
                m_ProjectThumbnails[projectPath] = thumb;
            } else {
                thumb = m_FallbackThumbnail;
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        
        ImVec2 cardStart = ImGui::GetCursorScreenPos();
        bool openProject = false;

        if (thumb && thumb->ID > 0) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
            openProject = ImGui::ImageButton("##ProjThumb", (ImTextureID)thumb->ID, ImVec2(cardWidth, cardHeight), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::PopStyleColor(2);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
            openProject = ImGui::Button(projectName.c_str(), ImVec2(cardWidth, cardHeight));
            ImGui::PopStyleColor(2);
        }

        if (openProject) {
            m_IsLoadingProject = true;
            m_CreationProgress = 0.0f;
            m_CreationStatus = "Opening project...";
            m_LoadingProjectPath = projectPath;
        }

        
        ImVec2 cardEnd = ImVec2(cardStart.x + cardWidth, cardStart.y + cardHeight);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float gradientH = 35.0f;
        dl->AddRectFilledMultiColor(
            ImVec2(cardStart.x, cardEnd.y - gradientH), cardEnd,
            ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0),
            ImColor(0, 0, 0, 200), ImColor(0, 0, 0, 200));
        
        ImVec2 namePos = ImVec2(cardStart.x + 8, cardEnd.y - 22);
        dl->AddText(ImGui::GetFont(), 14.0f, namePos, ImColor(1.0f, 1.0f, 1.0f, 0.95f), projectName.c_str());

        ImGui::PopStyleVar(); 

        
        float stripWidth = 35.0f;
        ImVec2 stripStart = ImVec2(cardStart.x + cardWidth - stripWidth, cardStart.y);
        ImVec2 stripEnd = ImVec2(cardStart.x + cardWidth, cardStart.y + cardHeight);
        
        bool stripHovered = ImGui::IsMouseHoveringRect(stripStart, stripEnd);
        
        
        dl->AddRectFilled(stripStart, stripEnd, 
            stripHovered ? ImColor(0.9f, 0.2f, 0.2f, 0.9f) : ImColor(0.7f, 0.15f, 0.15f, 0.5f), 
            6.0f, ImDrawFlags_RoundCornersRight);

        
        ImVec2 xCenter = ImVec2(stripStart.x + stripWidth / 2, stripStart.y + cardHeight / 2);
        float xSize = 8.0f;
        dl->AddLine(ImVec2(xCenter.x - xSize, xCenter.y - xSize), ImVec2(xCenter.x + xSize, xCenter.y + xSize), ImColor(1.0f, 1.0f, 1.0f, 0.8f), 2.0f);
        dl->AddLine(ImVec2(xCenter.x - xSize, xCenter.y + xSize), ImVec2(xCenter.x + xSize, xCenter.y - xSize), ImColor(1.0f, 1.0f, 1.0f, 0.8f), 2.0f);

        
        if (stripHovered && ImGui::IsMouseClicked(0)) {
            m_ProjectToDelete = projectPath;
            m_ShowDeletePopup = true;
        }

        
        if (stripHovered) {
            dl->AddRect(cardStart, ImVec2(cardStart.x + cardWidth, cardStart.y + cardHeight), 
                ImColor(0.9f, 0.1f, 0.1f, 0.8f), 6.0f, 0, 2.0f);
        }

        
        ImGui::SetCursorScreenPos(ImVec2(cardStart.x, cardStart.y + cardHeight + 4));
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", projectName.c_str());
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", projectPath.c_str());

        ImGui::PopID();
    }

    ImGui::EndTable();
  }
  ImGui::PopStyleVar(); 

  
  if (m_ShowNewProjectModal) {
      ImGui::OpenPopup("Create New Project");
      m_ShowNewProjectModal = false;
  }

  if (ImGui::BeginPopupModal("Create New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Spacing();
      ImGui::Text("Enter a name for your new project:");
      ImGui::Spacing();

      ImGui::SetNextItemWidth(350);
      ImGui::InputText("##NewProjName", m_ProjectName, sizeof(m_ProjectName));

      ImGui::Spacing();
      ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Location: %s", m_ProjectPath);
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.6f, 1.0f, 1.0f));
      if (ImGui::Button("Create Project", ImVec2(160, 36))) {
          if (strlen(m_ProjectName) > 0) {
              m_IsCreatingProject = true;
              m_CreationProgress = 0.0f;
              m_CreationStatus = "Initializing project structure...";
              ImGui::CloseCurrentPopup();
          }
      }
      ImGui::PopStyleColor(2);

      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(100, 36))) {
          ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
  }
}

void Home::RenderDemosView() {
  ImGui::SetWindowFontScale(1.1f);
  ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Sample Demo Scenes");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::Spacing();
  ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 0.8f), "Select a demo scene to preview, then open it in the editor.");
  ImGui::Spacing(); ImGui::Spacing();

  static std::vector<std::string> demoPaths;
  static std::vector<std::string> demoNames;

  if (demoPaths.empty()) {
    fs::path demosPath = fs::current_path().parent_path() / "Resource" / "Demos";
    if (fs::exists(demosPath)) {
      for (const auto &entry : fs::directory_iterator(demosPath)) {
        if (entry.is_directory()) {
          demoPaths.push_back(entry.path().string());
          demoNames.push_back(entry.path().filename().string());
        }
      }
    }
  }

  float cardWidth = 260.0f;
  float cardHeight = 180.0f;
  float cardSpacing = 15.0f;
  int columns = (int)(ImGui::GetContentRegionAvail().x / (cardWidth + cardSpacing));
  if (columns < 1) columns = 1;

  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(cardSpacing / 2, cardSpacing / 2));
  if (ImGui::BeginTable("TemplatesGrid", columns)) {
    for (int i = 0; i < (int)demoNames.size(); i++) {
      ImGui::TableNextColumn();
      ImGui::PushID(i + 200);

      std::string demoName = demoNames[i];
      std::string key = demoName;
      if (key.find("_") != std::string::npos)
        key = key.substr(key.find("_") + 1);

      
      Texture *previewTex = nullptr;
      if (key.find("Theater") != std::string::npos) previewTex = m_DemoPreviews["theater"];
      else if (key.find("Shadows") != std::string::npos) previewTex = m_DemoPreviews["lighting"];
      else if (key.find("Physics") != std::string::npos) previewTex = m_DemoPreviews["physics"];
      else if (key.find("Water") != std::string::npos) previewTex = m_DemoPreviews["water"];

      bool isSelected = (m_SelectedTemplateIdx == i);

      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

      ImVec2 cardStart = ImGui::GetCursorScreenPos();
      bool clicked = false;

      if (previewTex) {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
          clicked = ImGui::ImageButton("##TplBtn", (ImTextureID)previewTex->ID, ImVec2(cardWidth, cardHeight), ImVec2(0, 1), ImVec2(1, 0));
          ImGui::PopStyleColor(2);
      } else {
          ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
          ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.22f, 1.0f));
          clicked = ImGui::Button(key.c_str(), ImVec2(cardWidth, cardHeight));
          ImGui::PopStyleColor(2);
      }

      if (clicked) m_SelectedTemplateIdx = i;

      
      ImVec2 cardEnd = ImVec2(cardStart.x + cardWidth, cardStart.y + cardHeight);
      ImDrawList* dl = ImGui::GetWindowDrawList();
      float gradientH = 40.0f;
      dl->AddRectFilledMultiColor(
          ImVec2(cardStart.x, cardEnd.y - gradientH), cardEnd,
          ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0),
          ImColor(0, 0, 0, 210), ImColor(0, 0, 0, 210));
      ImVec2 namePos = ImVec2(cardStart.x + 10, cardEnd.y - 24);
      dl->AddText(ImGui::GetFont(), 14.0f, namePos, ImColor(1.0f, 1.0f, 1.0f, 0.95f), key.c_str());

      
      if (isSelected) {
          dl->AddRect(cardStart, ImVec2(cardStart.x + cardWidth + 8, cardStart.y + cardHeight + 8),
                      ImColor(0.35f, 0.65f, 1.0f, 0.9f), 6.0f, 0, 2.5f);
      }

      ImGui::PopStyleVar(); 

      
      ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.8f), "%s", demoName.c_str());

      ImGui::PopID();
    }
    ImGui::EndTable();
  }
  ImGui::PopStyleVar(); 
}

void Home::RenderSettingsView() {
  ImGui::SetWindowFontScale(1.1f);
  ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Engine Settings");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::Spacing();
  ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 0.8f), "Manage engine dependencies and configuration.");
  ImGui::Spacing(); ImGui::Spacing();

  
  ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.85f, 1.0f), "Core Dependencies");
  ImGui::Spacing();

  static std::vector<DependencyInfo> s_Deps = DependencyManager::GetDependencies();
  static float lastRefresh = 0;
  if (ImGui::GetTime() - lastRefresh > 5.0f) {
    s_Deps = DependencyManager::GetDependencies();
    lastRefresh = ImGui::GetTime();
  }

  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.1f, 1.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
  ImGui::BeginChild("DepList", ImVec2(0, 380), true);

  for (int i = 0; i < (int)s_Deps.size(); i++) {
    const auto &dep = s_Deps[i];
    ImGui::PushID(dep.packageName.c_str());

    
    if (i % 2 == 0) {
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRectFilled(
            rowPos, ImVec2(rowPos.x + ImGui::GetContentRegionAvail().x, rowPos.y + 30),
            ImColor(0.1f, 0.1f, 0.12f, 0.5f));
    }

    
    if (dep.isInstalled) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.45f, 0.2f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.45f, 0.2f, 1.0f));
      ImGui::SmallButton("Installed");
      ImGui::PopStyleColor(2);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.15f, 0.15f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.15f, 0.15f, 1.0f));
      ImGui::SmallButton("Missing ");
      ImGui::PopStyleColor(2);
    }

    ImGui::SameLine(130);
    ImGui::Text("%s", dep.name.c_str());
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("%s\nPackage: %s", dep.description.c_str(), dep.packageName.c_str());

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if (dep.isInstalled) {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.15f, 0.15f, 0.8f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.55f, 0.2f, 0.2f, 1.0f));
      if (ImGui::SmallButton("Remove")) {
        DependencyManager::UninstallPackage(dep);
        s_Deps = DependencyManager::GetDependencies();
      }
      ImGui::PopStyleColor(2);
    } else {
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.4f, 0.7f, 0.9f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.5f, 0.85f, 1.0f));
      if (ImGui::SmallButton("Install")) {
        DependencyManager::InstallPackage(dep);
        s_Deps = DependencyManager::GetDependencies();
      }
      ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar();

    ImGui::PopID();
    ImGui::Spacing();
  }
  ImGui::EndChild();
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();

  
  ImGui::Spacing(); ImGui::Spacing();
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.47f, 0.85f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.95f, 1.0f));
  if (ImGui::Button("Install All Missing", ImVec2(180, 36))) {
    DependencyManager::InstallAll();
    s_Deps = DependencyManager::GetDependencies();
  }
  ImGui::PopStyleColor(2);

  ImGui::SameLine(0, 15);
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));
  if (ImGui::Button("Run setup.sh", ImVec2(140, 36))) {
    system("./setup.sh &");
  }
  ImGui::PopStyleColor(2);

  ImGui::PopStyleVar();
}

void Home::LoadRecentProjects() {

  try {
    if (fs::exists(m_BaseProjectsPath)) {
      for (const auto &entry : fs::directory_iterator(m_BaseProjectsPath)) {
        if (entry.is_directory()) {
          m_RecentProjects.push_back(entry.path().string());
        }
      }
    }
  } catch (...) {
  }
}

void Home::SaveRecentProject(const std::string &path) {
  for (const auto &p : m_RecentProjects) {
    if (p == path)
      return;
  }
  m_RecentProjects.push_back(path);
}

void Home::DeleteProject(const std::string &path) {
  try {
    if (std::filesystem::exists(path)) {
      std::filesystem::remove_all(path);
      Logger::AddLog("[INFO] Deleted project: %s", path.c_str());
    }

    
    if (m_ProjectThumbnails.count(path)) {
      Texture* thumb = m_ProjectThumbnails[path];
      if (thumb && thumb != m_FallbackThumbnail) {
        thumb->Delete();
        delete thumb;
      }
      m_ProjectThumbnails.erase(path);
    }

    
    auto it = std::find(m_RecentProjects.begin(), m_RecentProjects.end(), path);
    if (it != m_RecentProjects.end()) {
      m_RecentProjects.erase(it);
    }
  } catch (const std::exception &e) {
    Logger::AddLog("[ERROR] Failed to delete project: %s", e.what());
  }
}

void Home::LoadDemoPreviews() {
  std::string basePath = "Resource/placeholder/";
  m_DemoPreviews["theater"] = new Texture((basePath + "theater.png").c_str());
  m_DemoPreviews["lighting"] = new Texture((basePath + "lighting.png").c_str());
  m_DemoPreviews["physics"] = new Texture((basePath + "physics.png").c_str());
  m_DemoPreviews["water"] = new Texture((basePath + "water.png").c_str());
  m_DemoPreviews["new_project"] = new Texture((basePath + "new_project.png").c_str());
  m_DemoPreviews["open_project"] = new Texture((basePath + "open_project.png").c_str());

  
  m_DemoPreviews["sidebar_recent"] = new Texture((basePath + "sidebar_recent.png").c_str());
  m_DemoPreviews["sidebar_demos"] = new Texture((basePath + "sidebar_demos.png").c_str());
  m_DemoPreviews["sidebar_settings"] = new Texture((basePath + "sidebar_settings.png").c_str());

  m_FallbackThumbnail = new Texture((basePath + "thumbnail_fallback.png").c_str());
}

void Home::RenderDemoDetails(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.09f, 0.09f, 0.11f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::BeginChild("TemplateDetails", ImVec2(width, height), true);
    
    if (m_SelectedTemplateIdx >= 0) {
        fs::path demosPath = fs::current_path().parent_path() / "Resource" / "Demos";
        std::vector<std::string> demoNames;
        std::vector<std::string> demoPaths;
        if (fs::exists(demosPath)) {
            for (const auto &entry : fs::directory_iterator(demosPath)) {
                if (entry.is_directory()) {
                    demoNames.push_back(entry.path().filename().string());
                    demoPaths.push_back(entry.path().string());
                }
            }
        }
        
        if (m_SelectedTemplateIdx < (int)demoNames.size()) {
            std::string name = demoNames[m_SelectedTemplateIdx];
            std::string demoPath = demoPaths[m_SelectedTemplateIdx];
            std::string key = name;
            if (key.find("_") != std::string::npos) key = key.substr(key.find("_") + 1);
            
            Texture* previewTex = nullptr;
            if (key.find("Theater") != std::string::npos) previewTex = m_DemoPreviews["theater"];
            else if (key.find("Shadows") != std::string::npos) previewTex = m_DemoPreviews["lighting"];
            else if (key.find("Physics") != std::string::npos) previewTex = m_DemoPreviews["physics"];
            else if (key.find("Water") != std::string::npos) previewTex = m_DemoPreviews["water"];

            
            ImGui::Spacing();
            if (previewTex) {
                ImGui::Image((ImTextureID)previewTex->ID, ImVec2(width - 30, 200), ImVec2(0, 1), ImVec2(1, 0));
            } else {
                ImGui::Dummy(ImVec2(width - 30, 200));
            }
            
            ImGui::Spacing(); ImGui::Spacing();

            
            ImGui::SetWindowFontScale(1.15f);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", key.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Spacing();

            
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + width - 30);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f),
                "A production-ready starting point with pre-configured lighting, physics, and world settings tailored for %s scenarios.", key.c_str());
            ImGui::PopTextWrapPos();
            
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            
            ImGui::TextColored(ImVec4(0.35f, 0.65f, 1.0f, 1.0f), "Project Defaults");
            ImGui::Spacing();
            
            static int platform = 0;
            const char* platforms[] = { "Linux / Desktop", "Windows (Cross-Compile)", "Android" };
            ImGui::SetNextItemWidth(width - 40);
            ImGui::Combo("##Platform", &platform, platforms, IM_ARRAYSIZE(platforms));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 0.7f), "Target Platform");
            ImGui::Spacing();
            
            static int quality = 1;
            const char* qualities[] = { "Low", "Medium", "High", "Ultra" };
            ImGui::SetNextItemWidth(width - 40);
            ImGui::Combo("##Quality", &quality, qualities, IM_ARRAYSIZE(qualities));
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 0.7f), "Quality Preset");

            ImGui::Spacing(); ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.47f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.95f, 1.0f));
            if (ImGui::Button("Open Demo Scene", ImVec2(width - 30, 36))) {
                m_IsLoadingProject = true;
                m_CreationProgress = 0.0f;
                m_CreationStatus = "Loading demo scene...";
                m_LoadingProjectPath = demoPath;
            }
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();
        }
    } else {
        
        float centerY = height * 0.4f;
        ImGui::SetCursorPos(ImVec2(20, centerY));
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.45f, 0.6f), "Select a template");
        ImGui::SetCursorPosX(20);
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.45f, 0.4f), "to view details and preview");
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void Home::RenderCreationBar(float width, float height) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.08f, 1.0f));
    ImGui::BeginChild("CreationBar", ImVec2(width, height), false);
    
    
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        pos, ImVec2(pos.x + width, pos.y),
        ImColor(0.2f, 0.2f, 0.25f, 1.0f), 1.0f);

    float yCenter = height * 0.5f - 12;
    float xCursor = 20.0f;

    
    ImGui::SetCursorPos(ImVec2(xCursor, yCenter - 12));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "LOCATION");
    ImGui::SetCursorPos(ImVec2(xCursor, yCenter + 6));
    float locWidth = width * 0.4f;
    ImGui::SetNextItemWidth(locWidth);
    ImGui::InputText("##ProjLoc", m_ProjectPath, sizeof(m_ProjectPath));

    ImGui::SameLine();
    ImGui::SetCursorPosY(yCenter + 4);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    if (ImGui::Button("Browse", ImVec2(70, 24))) {
        char buffer[256];
        std::string selectedPath = "";
        FILE *pipe = popen("zenity --file-selection --directory --title=\"Select Folder\"", "r");
        if (pipe) {
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) selectedPath += buffer;
            pclose(pipe);
            if (!selectedPath.empty() && selectedPath.back() == '\n') selectedPath.pop_back();
            if (!selectedPath.empty()) strncpy(m_ProjectPath, selectedPath.c_str(), sizeof(m_ProjectPath));
        }
    }
    ImGui::PopStyleColor(2);

    
    ImGui::SameLine(0, 30);
    float nameX = ImGui::GetCursorPosX();
    ImGui::SetCursorPos(ImVec2(nameX, yCenter - 12));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), "PROJECT NAME");
    ImGui::SetCursorPos(ImVec2(nameX, yCenter + 6));
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("##ProjName", m_ProjectName, sizeof(m_ProjectName));

    
    float btnWidth = 140.0f;
    float btnHeight = 44.0f;
    ImGui::SetCursorPos(ImVec2(width - btnWidth - 25, (height - btnHeight) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.47f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.95f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.8f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::Button("Create", ImVec2(btnWidth, btnHeight))) {
        if (strlen(m_ProjectName) > 0 && strlen(m_ProjectPath) > 0) {
            m_IsCreatingProject = true;
            m_CreationProgress = 0.0f;
            m_CreationStatus = "Initializing project structure...";
        }
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Home::RenderLoadingOverlay() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | 
                             ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.85f));
    ImGui::Begin("LoadingOverlay", nullptr, flags);
    
    ImVec2 center = ImVec2(viewport->Size.x * 0.5f, viewport->Size.y * 0.5f);
    
    
    ImGui::SetCursorPos(ImVec2(center.x - 100, center.y - 80));
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "CREATING PROJECT");
    
    
    float barWidth = 400.0f;
    float barHeight = 6.0f;
    ImVec2 barPos = ImVec2(center.x - barWidth * 0.5f, center.y);
    
    
    ImGui::GetWindowDrawList()->AddRectFilled(barPos, ImVec2(barPos.x + barWidth, barPos.y + barHeight), 
                                             ImColor(0.2f, 0.2f, 0.2f, 1.0f), 3.0f);
    
    
    float glowWidth = barWidth * m_CreationProgress;
    if (glowWidth > 0) {
        ImGui::GetWindowDrawList()->AddRectFilled(barPos, ImVec2(barPos.x + glowWidth, barPos.y + barHeight), 
                                                 ImColor(0.4f, 0.7f, 1.0f, 1.0f), 3.0f);
        
        
        for (int i = 0; i < 3; i++) {
            ImGui::GetWindowDrawList()->AddRect(ImVec2(barPos.x - i, barPos.y - i), 
                                                ImVec2(barPos.x + glowWidth + i, barPos.y + barHeight + i), 
                                                ImColor(0.4f, 0.7f, 1.0f, 0.15f), 3.0f);
        }
    }
    
    
    ImGui::SetCursorPos(ImVec2(center.x - ImGui::CalcTextSize(m_CreationStatus.c_str()).x * 0.5f, center.y + 30));
    ImGui::TextDisabled("%s", m_CreationStatus.c_str());
    
    
    char buf[16];
    sprintf(buf, "%d%%", (int)(m_CreationProgress * 100));
    ImGui::SetCursorPos(ImVec2(center.x + barWidth / 2 + 10, center.y - 5));
    ImGui::Text("%s", buf);

    ImGui::End();
    ImGui::PopStyleColor();
}
