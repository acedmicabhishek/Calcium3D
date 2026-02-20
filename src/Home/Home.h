#ifndef HOME_H
#define HOME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <filesystem>

class Home {
public:
    Home();
    ~Home();

    void Init(struct GLFWwindow* window);
    void Render();

private:
    struct GLFWwindow* m_Window = nullptr;
    std::filesystem::path m_BaseProjectsPath;
    std::vector<std::string> m_RecentProjects;
    
    void LoadRecentProjects();
    void SaveRecentProject(const std::string& path);
};

#endif
