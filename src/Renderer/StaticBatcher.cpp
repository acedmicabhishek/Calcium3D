#include "StaticBatcher.h"
#include "../Core/Logger.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

std::map<std::string, StaticBatcher::Batch> StaticBatcher::s_Batches;

std::string GetMaterialSignature(const Material &mat) {
  return mat.diffuseTexture + "|" + mat.specularTexture + "|" +
         mat.customShaderName + "|" + std::to_string(mat.albedo.r) +
         std::to_string(mat.albedo.g) + std::to_string(mat.albedo.b) + "|" +
         std::to_string(mat.metallic) + "|" + std::to_string(mat.roughness) +
         "|" + std::to_string(mat.isTransparent);
}

void StaticBatcher::Bake(Scene &scene) {
  Clear();
  auto &objects = scene.GetObjects();

  
  for (size_t i = 0; i < objects.size(); ++i) {
    auto &obj = objects[i];
    if (obj.isStatic && obj.isActive && obj.meshType != MeshType::Camera &&
        obj.meshType != MeshType::None) {
      std::string sig = GetMaterialSignature(obj.material);
      s_Batches[sig].material = obj.material;
      s_Batches[sig].originalObjectIndices.push_back((int)i);
    }
  }

  
  for (auto &pair : s_Batches) {
    std::vector<Vertex> batchVertices;
    std::vector<GLuint> batchIndices;
    std::vector<Texture> batchTextures; 

    GLuint indexOffset = 0;

    for (int idx : pair.second.originalObjectIndices) {
      auto &obj = objects[idx];
      glm::mat4 model = scene.GetGlobalTransform(idx);
      glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

      if (batchTextures.empty() && !obj.mesh.textures.empty()) {
        batchTextures = obj.mesh.textures;
      }

      
      for (const auto &v : obj.mesh.vertices) {
        Vertex worldVert = v;
        worldVert.position = glm::vec3(model * glm::vec4(v.position, 1.0f));
        worldVert.normal = glm::normalize(normalMatrix * v.normal);
        batchVertices.push_back(worldVert);
      }

      
      for (GLuint index : obj.mesh.indices) {
        batchIndices.push_back(index + indexOffset);
      }
      indexOffset += obj.mesh.vertices.size();

      
      obj.isActive = false;
    }

    if (!batchVertices.empty()) {
      pair.second.combinedMesh =
          new Mesh(batchVertices, batchIndices, batchTextures);
      Logger::AddLog("Baked static batch with %zu vertices, %zu indices",
                     batchVertices.size(), batchIndices.size());
    }
  }
}

void StaticBatcher::DrawBatches(Shader &shader, Camera &camera) {
  for (auto &pair : s_Batches) {
    if (!pair.second.combinedMesh)
      continue;

    const Material &mat = pair.second.material;
    shader.setVec3("material.albedo", mat.albedo);
    shader.setFloat("material.metallic", mat.metallic);
    shader.setFloat("material.roughness", mat.roughness);
    shader.setFloat("material.ao", mat.ao);
    shader.setInt("material.useTexture", mat.useTexture ? 1 : 0);
    shader.setInt("material.isTransparent", mat.isTransparent ? 1 : 0);

    
    glm::mat4 identity = glm::mat4(1.0f);
    shader.setMat4("model", identity);

    pair.second.combinedMesh->Draw(shader, camera, identity);
  }
}

void StaticBatcher::Clear() {
  for (auto &pair : s_Batches) {
    if (pair.second.combinedMesh) {
      pair.second.combinedMesh->Delete();
      delete pair.second.combinedMesh;
    }
  }
  s_Batches.clear();
}

bool StaticBatcher::HasBatches() { return !s_Batches.empty(); }
