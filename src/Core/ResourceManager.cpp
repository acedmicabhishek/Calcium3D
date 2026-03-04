#include "ResourceManager.h"
#include <iostream>
#include "Application.h"
#include <filesystem>

std::unordered_map<std::string, Shader> ResourceManager::Shaders;
std::unordered_map<std::string, Texture> ResourceManager::Textures;

std::string ResourceManager::ResolvePath(const std::string& path) {
    if (Application::Get().GetProjectRoot() != "") {
        std::filesystem::path projectShadersDir = std::filesystem::path(Application::Get().GetProjectRoot()) / "Shaders";
        std::filesystem::path targetFile = std::filesystem::path(path).filename();
        std::filesystem::path overridenPath = projectShadersDir / targetFile;
        
        if (std::filesystem::exists(overridenPath)) {
            return overridenPath.string();
        }
    }

#ifdef C3D_RUNTIME
    if (path.length() >= 11 && path.substr(0, 11) == "../shaders/") {
        return "Internal/shaders/" + path.substr(11);
    }
    if (path.length() >= 3 && path.substr(0, 3) == "../") {
        return "Internal/" + path.substr(3);
    }
#endif
    return path;
}

Shader& ResourceManager::LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    if (Shaders.find(name) != Shaders.end()) {
        return Shaders.at(name);
    }
    
    std::string vPath = ResolvePath(vShaderFile);
    std::string fPath = ResolvePath(fShaderFile);
    std::string gPath = gShaderFile != nullptr ? ResolvePath(gShaderFile) : "";
    
    if (gShaderFile != nullptr) {
        Shaders.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name),
                        std::forward_as_tuple(vPath.c_str(), fPath.c_str(), gPath.c_str()));
    } else {
        Shaders.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name),
                        std::forward_as_tuple(vPath.c_str(), fPath.c_str()));
    }
                    
    return Shaders.at(name);
}

Shader& ResourceManager::ReloadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
    std::string vPath = ResolvePath(vShaderFile);
    std::string fPath = ResolvePath(fShaderFile);
    std::string gPath = gShaderFile != nullptr ? ResolvePath(gShaderFile) : "";
    
    if (Shaders.find(name) != Shaders.end()) {
        Shaders.at(name).Delete(); 
        Shaders.erase(name);
    }
    
    if (gShaderFile != nullptr) {
        Shaders.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name),
                        std::forward_as_tuple(vPath.c_str(), fPath.c_str(), gPath.c_str()));
    } else {
        Shaders.emplace(std::piecewise_construct,
                        std::forward_as_tuple(name),
                        std::forward_as_tuple(vPath.c_str(), fPath.c_str()));
    }
                    
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name) {
    if (Shaders.find(name) == Shaders.end()) {
        std::cerr << "ResourceManager: Shader not found: " << name << std::endl;
        return Shaders.begin()->second; 
    }
    return Shaders.at(name);
}

bool ResourceManager::HasShader(const std::string& name) {
    return Shaders.find(name) != Shaders.end();
}

Texture& ResourceManager::LoadTexture(const std::string& name, const char* file, const char* texType, GLuint slot) {
    if (Textures.find(name) != Textures.end()) {
        return Textures.at(name);
    }
    
    std::string path = ResolvePath(file);
    Textures.emplace(std::piecewise_construct,
                     std::forward_as_tuple(name),
                     std::forward_as_tuple(path.c_str(), texType, slot));
                     
    return Textures.at(name);
}

Texture& ResourceManager::GetTexture(const std::string& name) {
    if (Textures.find(name) == Textures.end()) {
        std::cerr << "ResourceManager: Texture not found: " << name << std::endl;
        return Textures.begin()->second;
    }
    return Textures.at(name);
}

bool ResourceManager::HasTexture(const std::string& name) {
    return Textures.find(name) != Textures.end();
}

void ResourceManager::Clear() {
    for (auto& iter : Shaders)
        iter.second.Delete();
    for (auto& iter : Textures)
        iter.second.Delete();
        
    Shaders.clear();
    Textures.clear();
}
