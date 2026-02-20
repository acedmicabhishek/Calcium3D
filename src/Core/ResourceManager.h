#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <unordered_map>
#include <string>
#include "Shader.h"
#include "Texture.h"
#include "Model.h"

class ResourceManager {
public:
    static Shader& LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile);
    static Shader& GetShader(const std::string& name);
    
    static std::string ResolvePath(const std::string& path);
    
    static Texture& LoadTexture(const std::string& name, const char* file, const char* texType, GLuint slot);
    static Texture& GetTexture(const std::string& name);
    
    static void Clear();

private:
    static std::unordered_map<std::string, Shader> Shaders;
    static std::unordered_map<std::string, Texture> Textures;
    
    ResourceManager() {}
};

#endif
