#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include "Camera.h"
#include "EBO.h"
#include "Texture.h"
#include "VAO.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  std::vector<Texture> textures;
  VAO vao;
  GLuint vboID;
  GLuint eboID;
  glm::vec3 minAABB;
  glm::vec3 maxAABB;
  Mesh() : vboID(0), eboID(0) {}

  Mesh(const std::vector<Vertex> &vertices, const std::vector<GLuint> &indices,
       std::vector<Texture> textures);
  void Draw(Shader &shader, Camera &camera,
            glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
            unsigned int textureOverride = 0) const;

  void Draw(Shader &shader, Camera &camera, const glm::mat4 &matrix,
            unsigned int textureOverride = 0) const;

  void UpdateVBO();
  void Delete();

  bool Intersect(const glm::vec3 &ray_origin, const glm::vec3 &ray_direction,
                 const glm::mat4 &modelMatrix, float &intersection_distance);
  void RemapUVs(const glm::vec2 &offset, const glm::vec2 &scale);

  struct LODLevel {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
  };

  std::vector<LODLevel> lodLevels;
  int currentLOD = 0;

  void GenerateLODs();
};
#endif