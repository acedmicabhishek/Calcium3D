#include "ResourceManager.h"
#include <iostream>

std::unordered_map<std::string, Shader> ResourceManager::Shaders;
std::unordered_map<std::string, Texture> ResourceManager::Textures;

Shader& ResourceManager::LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile) {
    if (Shaders.find(name) != Shaders.end()) {
        return Shaders.at(name);
    }
    
    Shaders.emplace(std::piecewise_construct,
                    std::forward_as_tuple(name),
                    std::forward_as_tuple(vShaderFile, fShaderFile));
                    
    return Shaders.at(name);
}

Shader& ResourceManager::GetShader(const std::string& name) {
    if (Shaders.find(name) == Shaders.end()) {
        std::cerr << "ResourceManager: Shader not found: " << name << std::endl;
        return Shaders.begin()->second; 
    }
    return Shaders.at(name);
}

Texture& ResourceManager::LoadTexture(const std::string& name, const char* file, const char* texType, GLuint slot) {
    if (Textures.find(name) != Textures.end()) {
        return Textures.at(name);
    }
    
    Textures.emplace(std::piecewise_construct,
                     std::forward_as_tuple(name),
                     std::forward_as_tuple(file, texType, slot));
                     
    return Textures.at(name);
}

Texture& ResourceManager::GetTexture(const std::string& name) {
    if (Textures.find(name) == Textures.end()) {
        std::cerr << "ResourceManager: Texture not found: " << name << std::endl;
        return Textures.begin()->second;
    }
    return Textures.at(name);
}

void ResourceManager::Clear() {
    for (auto& iter : Shaders)
        iter.second.Delete();
    for (auto& iter : Textures)
        iter.second.Delete();
        
    Shaders.clear();
    Textures.clear();
}
