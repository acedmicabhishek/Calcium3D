#ifndef HOME_H
#define HOME_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

struct GLFWwindow;
class Texture;

enum class HomeTab { Projects, Demos, Settings };

class Home {
public:
    Home();
    ~Home();

  void Init(struct GLFWwindow *window);
  void Update(float deltaTime);
  void Render();

private:
  struct GLFWwindow *m_Window = nullptr;
  std::filesystem::path m_BaseProjectsPath;
  std::vector<std::string> m_RecentProjects;
  std::string m_ProjectToDelete;
  HomeTab m_CurrentTab = HomeTab::Projects;

  std::map<std::string, Texture *> m_DemoPreviews;
  std::map<std::string, Texture *> m_ProjectThumbnails;
  Texture *m_FallbackThumbnail = nullptr;

  void LoadRecentProjects();
  void SaveRecentProject(const std::string &path);
  void DeleteProject(const std::string &path);
  void LoadDemoPreviews();

  
  void RenderSidebar(float width, float height);
  void RenderProjectsView();
  void RenderDemosView();
  void RenderSettingsView();
  void RenderDemoDetails(float width, float height);
  void RenderCreationBar(float width, float height);
  void RenderLoadingOverlay();

  int m_SelectedTemplateIdx = -1;
  char m_ProjectName[128] = "MyProject";
  char m_ProjectPath[256];

  bool m_IsCreatingProject = false;
  float m_CreationProgress = 0.0f;
  std::string m_CreationStatus = "";
  bool m_ShowNewProjectModal = false;

  
  bool m_IsLoadingProject = false;
  std::string m_LoadingProjectPath = "";
  bool m_ShowDeletePopup = false;
};

#endif
