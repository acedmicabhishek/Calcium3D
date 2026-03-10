#include "Mesh.h"
#include "../Core/Logger.h"
#include "LODGenerator.h"
#include <limits>

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<GLuint> &indices, std::vector<Texture> textures) {
  Mesh::vertices = vertices;
  Mesh::indices = indices;
  Mesh::textures = std::move(textures);

  minAABB = glm::vec3(std::numeric_limits<float>::max());
  maxAABB = glm::vec3(std::numeric_limits<float>::lowest());

  for (const auto &vertex : vertices) {
    minAABB.x = std::min(minAABB.x, vertex.position.x);
    minAABB.y = std::min(minAABB.y, vertex.position.y);
    minAABB.z = std::min(minAABB.z, vertex.position.z);

    maxAABB.x = std::max(maxAABB.x, vertex.position.x);
    maxAABB.y = std::max(maxAABB.y, vertex.position.y);
    maxAABB.z = std::max(maxAABB.z, vertex.position.z);
  }

  vao.Bind();
  VBO VBO(vertices);
  vboID = VBO.ID;
  EBO EBO(indices);
  eboID = EBO.ID;

  vao.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex),
                 (void *)offsetof(Vertex, position));
  vao.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex),
                 (void *)offsetof(Vertex, color));
  vao.LinkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex),
                 (void *)offsetof(Vertex, texUV));
  vao.LinkAttrib(VBO, 3, 3, GL_FLOAT, sizeof(Vertex),
                 (void *)offsetof(Vertex, normal));

  vao.LinkAttrib(VBO, 3, 3, GL_FLOAT, sizeof(Vertex),
                 (void *)offsetof(Vertex, normal));

  vao.Unbind();
  VBO.Unbind();
  EBO.Unbind();

  if (indices.size() > 900) {
    GenerateLODs();
  }
}

void Mesh::GenerateLODs() {
  lodLevels.clear();

  float targetRatios[] = {0.5f, 0.05f, 0.01f, 0.002f};

  std::vector<Vertex> currentVerts = vertices;
  std::vector<GLuint> currentInds = indices;

  for (int i = 0; i < 4; ++i) {
    LODLevel lod;

    LODGenerator::SimplifyMesh(vertices, indices, targetRatios[i], lod.vertices,
                               lod.indices);

    if (lod.indices.size() <
            (i == 0 ? indices.size() : lodLevels.back().indices.size()) &&
        !lod.indices.empty()) {
      glGenVertexArrays(1, &lod.vao);
      glGenBuffers(1, &lod.vbo);
      glGenBuffers(1, &lod.ebo);

      glBindVertexArray(lod.vao);

      glBindBuffer(GL_ARRAY_BUFFER, lod.vbo);
      glBufferData(GL_ARRAY_BUFFER, lod.vertices.size() * sizeof(Vertex),
                   lod.vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lod.ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, lod.indices.size() * sizeof(GLuint),
                   lod.indices.data(), GL_STATIC_DRAW);

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

      glBindVertexArray(0);

      lodLevels.push_back(lod);

    } else {
      break;
    }
  }

  if (!lodLevels.empty()) {
    Logger::AddLog("Generated %zu LOD levels for mesh containing %zu indices.",
                   lodLevels.size(), indices.size());
  }
}

void Mesh::UpdateVBO() {
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::Delete() {
  vao.Delete();
  if (vboID != 0)
    glDeleteBuffers(1, &vboID);
  if (eboID != 0)
    glDeleteBuffers(1, &eboID);
  vboID = 0;
  eboID = 0;
  vertices.clear();
  indices.clear();
}

void Mesh::Draw(Shader &shader, Camera &camera, glm::vec3 position,
                glm::quat rotation, glm::vec3 scale,
                unsigned int textureOverride) const {
  shader.use();
  vao.Bind();
  unsigned int numDiffuse = 0;
  unsigned int numSpecular = 0;
  if (textureOverride != 0) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOverride);
    shader.setInt("diffuse0", 0);
  } else {
    for (unsigned int i = 0; i < textures.size(); i++) {
      std::string num;
      std::string type = textures[i].type;
      if (type == "diffuse") {
        num = std::to_string(numDiffuse++);
      } else if (type == "specular") {
        num = std::to_string(numSpecular++);
      }
      textures[i].texUnit(shader, (type + num).c_str(), textures[i].unit);
      textures[i].Bind();
    }
  }
  glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x,
              camera.Position.y, camera.Position.z);
  glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), camera.Position.x,
              camera.Position.y, camera.Position.z);
  camera.Matrix(camera.FOV, camera.nearPlane, camera.farPlane, shader,
                "camMatrix");

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model *= glm::mat4_cast(rotation);
  model = glm::scale(model, scale);

  glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));

  if (currentLOD > 0 && currentLOD <= lodLevels.size()) {
    glBindVertexArray(lodLevels[currentLOD - 1].vao);
    glDrawElements(GL_TRIANGLES, lodLevels[currentLOD - 1].indices.size(),
                   GL_UNSIGNED_INT, 0);
  } else {
    vao.Bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}

void Mesh::Draw(Shader &shader, Camera &camera, const glm::mat4 &model,
                unsigned int textureOverride) const {
  shader.use();

  unsigned int numDiffuse = 0;
  unsigned int numSpecular = 0;
  if (textureOverride != 0) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureOverride);
    shader.setBool("material.useTexture", true);
    shader.setInt("diffuse0", 0);
  } else {
    for (unsigned int i = 0; i < textures.size(); i++) {
      std::string num;
      std::string type = textures[i].type;
      if (type == "diffuse") {
        num = std::to_string(numDiffuse++);
      } else if (type == "specular") {
        num = std::to_string(numSpecular++);
      }
      textures[i].texUnit(shader, (type + num).c_str(), textures[i].unit);
      textures[i].Bind();
    }
  }
  glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x,
              camera.Position.y, camera.Position.z);
  glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), camera.Position.x,
              camera.Position.y, camera.Position.z);
  camera.Matrix(camera.FOV, camera.nearPlane, camera.farPlane, shader,
                "camMatrix");

  glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE,
                     glm::value_ptr(model));

  float scaleX = glm::length(glm::vec3(model[0]));
  float scaleY = glm::length(glm::vec3(model[1]));
  float scaleZ = glm::length(glm::vec3(model[2]));
  glUniform3f(glGetUniformLocation(shader.ID, "tilingFactor"), scaleX, scaleY,
              scaleZ);

  glUniform3f(glGetUniformLocation(shader.ID, "tilingFactor"), scaleX, scaleY,
              scaleZ);

  if (currentLOD > 0 && currentLOD <= lodLevels.size()) {
    glBindVertexArray(lodLevels[currentLOD - 1].vao);
    glDrawElements(GL_TRIANGLES, lodLevels[currentLOD - 1].indices.size(),
                   GL_UNSIGNED_INT, 0);
  } else {
    vao.Bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}

bool Mesh::Intersect(const glm::vec3 &ray_origin,
                     const glm::vec3 &ray_direction,
                     const glm::mat4 &modelMatrix,
                     float &intersection_distance) {
  glm::mat4 invModelMatrix = glm::inverse(modelMatrix);
  glm::vec3 localRayOrigin =
      glm::vec3(invModelMatrix * glm::vec4(ray_origin, 1.0f));
  glm::vec3 localRayDirection =
      glm::vec3(invModelMatrix * glm::vec4(ray_direction, 0.0f));
  localRayDirection = glm::normalize(localRayDirection);

  float tmin = 0.0;
  float tmax = std::numeric_limits<float>::max();

  for (int i = 0; i < 3; ++i) {
    if (abs(localRayDirection[i]) < 1e-6) {
      if (localRayOrigin[i] < minAABB[i] || localRayOrigin[i] > maxAABB[i]) {
        return false;
      }
    } else {
      float ood = 1.0f / localRayDirection[i];
      float t1 = (minAABB[i] - localRayOrigin[i]) * ood;
      float t2 = (maxAABB[i] - localRayOrigin[i]) * ood;

      if (t1 > t2) {
        std::swap(t1, t2);
      }

      tmin = std::max(tmin, t1);
      tmax = std::min(tmax, t2);

      if (tmin > tmax) {
        return false;
      }
    }
  }

  glm::vec3 intersectionPoint = localRayOrigin + localRayDirection * tmin;
  glm::vec3 worldIntersectionPoint =
      glm::vec3(modelMatrix * glm::vec4(intersectionPoint, 1.0f));
  intersection_distance = glm::length(worldIntersectionPoint - ray_origin);

  return true;
}

void Mesh::RemapUVs(const glm::vec2 &offset, const glm::vec2 &scale) {
  
  for (auto &v : vertices) {
    v.texUV = offset + v.texUV * scale;
  }
  UpdateVBO();

  
  for (auto &lod : lodLevels) {
    for (auto &v : lod.vertices) {
      v.texUV = offset + v.texUV * scale;
    }
    glBindBuffer(GL_ARRAY_BUFFER, lod.vbo);
    glBufferData(GL_ARRAY_BUFFER, lod.vertices.size() * sizeof(Vertex),
                 lod.vertices.data(), GL_STATIC_DRAW);
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}