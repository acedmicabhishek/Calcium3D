#include "SDFGenerator.h"
#include <Core/Logger.h>
#include <algorithm>
#include <glad/glad.h>
#include <limits>

SDFVolume SDFGenerator::GenerateSDF(const Mesh &mesh, int resolution) {
  Logger::AddLog("[SDF] Generating %dx%dx%d volume...", resolution, resolution,
                 resolution);

  std::vector<float> data(resolution * resolution * resolution);
  glm::vec3 minP = mesh.minAABB;
  glm::vec3 maxP = mesh.maxAABB;

  
  glm::vec3 size = maxP - minP;
  minP -= size * 0.1f;
  maxP += size * 0.1f;
  size = maxP - minP;

  for (int z = 0; z < resolution; z++) {
    for (int y = 0; y < resolution; y++) {
      for (int x = 0; x < resolution; x++) {
        glm::vec3 samplePos = minP + size * glm::vec3((float)x / resolution,
                                                      (float)y / resolution,
                                                      (float)z / resolution);
        data[x + y * resolution + z * resolution * resolution] =
            CalculateDistanceToMesh(samplePos, mesh);
      }
    }
  }

  unsigned int texID;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_3D, texID);
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, resolution, resolution, resolution, 0,
               GL_RED, GL_FLOAT, data.data());

  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return {texID, minP, maxP, resolution};
}

float SDFGenerator::CalculateDistanceToMesh(const glm::vec3 &samplePoint,
                                            const Mesh &mesh) {
  float minDist = std::numeric_limits<float>::max();

  
  for (size_t i = 0; i < mesh.indices.size(); i += 3) {
    glm::vec3 v0 = mesh.vertices[mesh.indices[i]].position;
    glm::vec3 v1 = mesh.vertices[mesh.indices[i + 1]].position;
    glm::vec3 v2 = mesh.vertices[mesh.indices[i + 2]].position;

    
    glm::vec3 center = (v0 + v1 + v2) / 3.0f;
    float d = glm::distance(samplePoint, center);
    if (d < minDist)
      minDist = d;
  }

  
  
  
  return minDist;
}
