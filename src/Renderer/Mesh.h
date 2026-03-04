#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../AniEngine/Animation.h"
#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    VAO vao;
    GLuint vboID;
    glm::vec3 minAABB;
    glm::vec3 maxAABB;
    
    
    Skeleton skeleton;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, std::vector<Texture> textures);

    void Draw(Shader& shader, Camera& camera, 
              glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
              glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), 
              glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
              unsigned int textureOverride = 0,
              const std::vector<glm::mat4>& boneMatrices = {});
              
    void Draw(Shader& shader, Camera& camera, const glm::mat4& matrix, unsigned int textureOverride = 0, const std::vector<glm::mat4>& boneMatrices = {});
              
    void UpdateVBO();
              
    bool Intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::mat4& modelMatrix, float& intersectionDistance);
};

#endif