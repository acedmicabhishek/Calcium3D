#include "DependencyManager.h"
#include <cstdlib>
#include <cstdio>
#include <algorithm>

#ifdef __linux__
#include <unistd.h>
#endif

bool DependencyManager::IsInstalled(const std::string& packageName) {
    char cmd[256];
    
    snprintf(cmd, sizeof(cmd), "pkg-config --exists %s 2>/dev/null", packageName.c_str());
    if (system(cmd) == 0) return true;
    
    
    snprintf(cmd, sizeof(cmd), "which %s > /dev/null 2>&1", packageName.c_str());
    if (system(cmd) == 0) return true;
    
    return false;
}

std::vector<DependencyInfo> DependencyManager::GetDependencies() {
    std::vector<DependencyInfo> deps = {
        {"GLFW3", "glfw3", "glfw-x11", "libglfw3-dev", false, "OpenGL Windowing Library"},
        {"GLEW", "glew", "glew", "libglew-dev", false, "OpenGL Extension Wrangler"},
        {"GLM", "glm", "glm", "libglm-dev", false, "OpenGL Mathematics Library"},
        {"JSON", "nlohmann_json", "nlohmann-json", "nlohmann-json3-dev", false, "JSON for Modern C++"},
        {"FFmpeg", "libavcodec", "ffmpeg", "libavcodec-dev libavformat-dev libswscale-dev libavutil-dev", false, "Video Encoding/Decoding"},
        {"Zenity", "zenity", "zenity", "zenity", false, "Native File Dialogs"},
        {"PkgConfig", "pkg-config", "pkgconf", "pkg-config", false, "Dependency Management Tool"}
    };
    
    for (auto& dep : deps) {
        dep.isInstalled = IsInstalled(dep.packageName);
    }
    
    return deps;
}

std::string DependencyManager::GetPackageManager() {
    if (system("which pacman > /dev/null 2>&1") == 0) return "pacman";
    if (system("which apt-get > /dev/null 2>&1") == 0) return "apt";
    return "";
}

bool DependencyManager::RunWithSudo(const std::string& command) {
    
    std::string pkCmd = "pkexec " + command;
    printf("[DependencyManager] Trying pkexec: %s\n", pkCmd.c_str());
    if (system(pkCmd.c_str()) == 0) return true;
    
    
    return system(command.c_str()) == 0;
}

bool DependencyManager::InstallPackage(const DependencyInfo& dep) {
    std::string pm = GetPackageManager();
    if (pm == "pacman") {
        return RunWithSudo("pacman -S --noconfirm " + dep.pacmanPackage);
    } else if (pm == "apt") {
        return RunWithSudo("apt-get update && apt-get install -y " + dep.aptPackage);
    }
    return false;
}

bool DependencyManager::UninstallPackage(const DependencyInfo& dep) {
    std::string pm = GetPackageManager();
    if (pm == "pacman") {
        return RunWithSudo("pacman -Rs --noconfirm " + dep.pacmanPackage);
    } else if (pm == "apt") {
        return RunWithSudo("apt-get remove -y " + dep.aptPackage);
    }
    return false;
}

bool DependencyManager::InstallAll() {
    auto deps = GetDependencies();
    bool success = true;
    for (const auto& dep : deps) {
        if (!dep.isInstalled) {
            if (!InstallPackage(dep)) success = false;
        }
    }
    return success;
}
