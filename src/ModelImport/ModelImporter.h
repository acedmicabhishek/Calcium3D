#ifndef MODEL_IMPORTER_H
#define MODEL_IMPORTER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../Renderer/VBO.h"
#include "../Renderer/Texture.h"
#include "../AniEngine/Animation.h"

struct ImportedMeshData {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    std::string name;

    glm::vec3 albedo = glm::vec3(0.8f);
    float metallic = 0.0f;
    float roughness = 0.5f;

    
    Skeleton skeleton;
};

struct ImportResult {
    bool success = false;
    std::string error;
    std::vector<ImportedMeshData> meshes;
    std::vector<AnimationClip> animations;
    std::string sourceFile;
};

class ModelImporter {
public:
    static ImportResult Import(const std::string& filepath);

    static bool IsModelFile(const std::string& extension);

    static std::vector<std::string> GetSupportedExtensions();

private:
    static ImportResult ImportOBJ(const std::string& filepath);
    static ImportResult ImportFBX(const std::string& filepath);
    static ImportResult ImportGLTF(const std::string& filepath);
    static ImportResult ImportSTL(const std::string& filepath);
    static ImportResult ImportPLY(const std::string& filepath);
};

#endif
