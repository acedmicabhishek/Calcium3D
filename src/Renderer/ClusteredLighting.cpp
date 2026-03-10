#include "ClusteredLighting.h"
#include "Tools/Profiler/Profiler.h"
#include <algorithm>
#include <cmath>
#include <glad/glad.h>
#include <iostream>

#ifndef GL_SHADER_STORAGE_BUFFER
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#endif

unsigned int ClusteredLighting::s_LightSSBO = 0;
unsigned int ClusteredLighting::s_ClusterSSBO = 0;
bool ClusteredLighting::s_Initialized = false;
std::vector<ClusteredLighting::PointLightShaderData>
    ClusteredLighting::s_LightBuffer;
std::vector<ClusteredLighting::ClusterData> ClusteredLighting::s_ClusterBuffer;

float squaredDistPointAABB(const glm::vec3 &point, const glm::vec3 &min,
                           const glm::vec3 &max) {
  float sqDist = 0.0f;
  for (int i = 0; i < 3; i++) {
    float v = point[i];
    if (v < min[i])
      sqDist += (min[i] - v) * (min[i] - v);
    if (v > max[i])
      sqDist += (v - max[i]) * (v - max[i]);
  }
  return sqDist;
}

void ClusteredLighting::Init() {
  if (s_Initialized)
    return;

  s_ClusterBuffer.resize(TOTAL_CLUSTERS);

  glGenBuffers(1, &s_LightSSBO);
  glGenBuffers(1, &s_ClusterSSBO);

  
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ClusterSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ClusterData) * TOTAL_CLUSTERS,
               nullptr, GL_DYNAMIC_DRAW);

  s_Initialized = true;
}

void ClusteredLighting::UpdateClusters(Camera &camera, Scene &scene) {
  PROFILE_SCOPE("ClusteredLighting_Update");
  if (!s_Initialized)
    Init();

  
  for (auto &cluster : s_ClusterBuffer) {
    cluster.count = 0;
  }
  s_LightBuffer.clear();

  
  float zNear = camera.nearPlane;
  float zFar = camera.farPlane;

  
  float zScale = (float)CLUSTER_Z / std::log2(zFar / zNear);
  float zBias =
      -((float)CLUSTER_Z * std::log2(zNear) / std::log2(zFar / zNear));

  
  glm::mat4 view = camera.GetViewMatrix();

  auto &pointLights = scene.GetPointLights();
  int lightIndex = 0;

  for (size_t i = 0; i < pointLights.size(); ++i) {
    auto &ptLight = pointLights[i];
    if (!ptLight.enabled)
      continue;

    glm::vec3 worldPos = ptLight.position;
    glm::vec3 viewPos = glm::vec3(view * glm::vec4(worldPos, 1.0f));

    
    
    float maxRange = (-ptLight.linear +
                      std::sqrt(ptLight.linear * ptLight.linear -
                                4 * ptLight.quadratic *
                                    (ptLight.constant - (256.0f / 5.0f)))) /
                     (2 * ptLight.quadratic);

    PointLightShaderData pl;
    pl.positionAndRadius = glm::vec4(worldPos, maxRange);
    pl.colorAndIntensity = glm::vec4(ptLight.color.r, ptLight.color.g,
                                     ptLight.color.b, ptLight.intensity);
    s_LightBuffer.push_back(pl);

    
    float minZ = -viewPos.z - maxRange;
    float maxZ = -viewPos.z + maxRange;

    if (minZ < zNear)
      minZ = zNear;
    if (maxZ > zFar)
      maxZ = zFar;
    if (maxZ < zNear || minZ > zFar) {
      lightIndex++;
      continue;
    }

    int startZ =
        std::clamp((int)(std::log2(minZ) * zScale + zBias), 0, CLUSTER_Z - 1);
    int endZ =
        std::clamp((int)(std::log2(maxZ) * zScale + zBias), 0, CLUSTER_Z - 1);

    
    
    
    
    glm::vec4 p1 = camera.GetProjectionMatrix() *
                   glm::vec4(viewPos.x - maxRange, viewPos.y - maxRange,
                             viewPos.z + maxRange, 1.0);
    glm::vec4 p2 = camera.GetProjectionMatrix() *
                   glm::vec4(viewPos.x + maxRange, viewPos.y + maxRange,
                             viewPos.z - maxRange, 1.0);

    p1 /= p1.w;
    p2 /= p2.w;

    int startX =
        std::clamp((int)((p1.x + 1.0f) * 0.5f * CLUSTER_X), 0, CLUSTER_X - 1);
    int endX =
        std::clamp((int)((p2.x + 1.0f) * 0.5f * CLUSTER_X), 0, CLUSTER_X - 1);
    int startY =
        std::clamp((int)((p1.y + 1.0f) * 0.5f * CLUSTER_Y), 0, CLUSTER_Y - 1);
    int endY =
        std::clamp((int)((p2.y + 1.0f) * 0.5f * CLUSTER_Y), 0, CLUSTER_Y - 1);

    
    for (int z = startZ; z <= endZ; ++z) {
      for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
          int clusterIdx = x + (y * CLUSTER_X) + (z * CLUSTER_X * CLUSTER_Y);
          if (clusterIdx >= 0 && clusterIdx < TOTAL_CLUSTERS) {
            ClusterData &cluster = s_ClusterBuffer[clusterIdx];
            if (cluster.count < MAX_LIGHTS_PER_CLUSTER) {
              cluster.lightIndices[cluster.count++] = lightIndex;
            }
          }
        }
      }
    }

    lightIndex++;
  }

  
  if (!s_LightBuffer.empty()) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_LightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 s_LightBuffer.size() * sizeof(PointLightShaderData),
                 s_LightBuffer.data(), GL_DYNAMIC_DRAW);
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_ClusterSSBO);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
                  TOTAL_CLUSTERS * sizeof(ClusterData), s_ClusterBuffer.data());
}

void ClusteredLighting::BindBuffers(unsigned int lightBindingPoint) {
  if (!s_Initialized)
    return;
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, lightBindingPoint, s_LightSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, lightBindingPoint + 1,
                   s_ClusterSSBO);
}

void ClusteredLighting::Shutdown() {
  if (s_LightSSBO)
    glDeleteBuffers(1, &s_LightSSBO);
  if (s_ClusterSSBO)
    glDeleteBuffers(1, &s_ClusterSSBO);
  s_Initialized = false;
}
