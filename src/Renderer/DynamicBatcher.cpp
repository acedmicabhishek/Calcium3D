#include "DynamicBatcher.h"
#include "../Core/Logger.h"
#include "StaticBatcher.h" 
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>


extern std::string GetMaterialSignature(const Material &mat);

void DynamicBatcher::DrawBatches(Scene &scene, Shader &shader, Camera &camera) {
  auto &objects = scene.GetObjects();
  std::map<std::string, DynamicBatch> batches;

  
  for (size_t i = 0; i < objects.size(); ++i) {
    auto &obj = objects[i];
    if (!obj.isStatic && obj.isActive && obj.meshType != MeshType::Camera &&
        obj.meshType != MeshType::None) {
      
      if (obj.mesh.vertices.size() < 300) {
        std::string sig = GetMaterialSignature(obj.material);
        auto &batch = batches[sig];

        if (batch.vertices.empty()) {
          batch.material = obj.material;
          if (!obj.mesh.textures.empty()) {
            batch.textures = obj.mesh.textures;
          }
        }

        glm::mat4 model = scene.GetGlobalTransform(i);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

        GLuint indexOffset = batch.vertices.size();

        
        for (const auto &v : obj.mesh.vertices) {
          Vertex worldVert = v;
          worldVert.position = glm::vec3(model * glm::vec4(v.position, 1.0f));
          worldVert.normal = glm::normalize(normalMatrix * v.normal);
          batch.vertices.push_back(worldVert);
        }

        
        for (GLuint index : obj.mesh.indices) {
          batch.indices.push_back(index + indexOffset);
        }

        
        
      }
    }
  }

  
  if (batches.empty())
    return;

  
  GLuint vao, vbo, ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  for (auto &pair : batches) {
    auto &batch = pair.second;
    if (batch.vertices.empty())
      continue;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, batch.vertices.size() * sizeof(Vertex),
                 batch.vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch.indices.size() * sizeof(GLuint),
                 batch.indices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, texUV));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *)offsetof(Vertex, normal));

    const Material &mat = batch.material;
    shader.setVec3("material.albedo", mat.albedo);
    shader.setFloat("material.metallic", mat.metallic);
    shader.setFloat("material.roughness", mat.roughness);
    shader.setFloat("material.ao", mat.ao);
    shader.setInt("material.useTexture", mat.useTexture ? 1 : 0);
    shader.setInt("material.isTransparent", mat.isTransparent ? 1 : 0);

    
    glm::mat4 identity = glm::mat4(1.0f);
    shader.setMat4("model", identity);

    
    if (mat.useTexture && !batch.textures.empty()) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, batch.textures[0].ID);
      shader.setInt("diffuse0", 0);
    }

    glDrawElements(GL_TRIANGLES, batch.indices.size(), GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}
