#ifndef CLUSTERED_LIGHTING_H
#define CLUSTERED_LIGHTING_H

#include "../Scene/Scene.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <vector>

constexpr int CLUSTER_X = 16;
constexpr int CLUSTER_Y = 9;
constexpr int CLUSTER_Z = 24;
constexpr int TOTAL_CLUSTERS = CLUSTER_X * CLUSTER_Y * CLUSTER_Z;
constexpr int MAX_LIGHTS_PER_CLUSTER = 64;

class ClusteredLighting {
public:
  static void Init();
  static void UpdateClusters(Camera &camera, Scene &scene);
  static void BindBuffers(unsigned int bindingPoint);
  static void Shutdown();

private:
  struct PointLightShaderData {
    glm::vec4 positionAndRadius;
    glm::vec4 colorAndIntensity;
  };

  struct ClusterData {
    int count;
    int lightIndices[MAX_LIGHTS_PER_CLUSTER];
  };

  static unsigned int s_LightSSBO;
  static unsigned int s_ClusterSSBO;
  static bool s_Initialized;

  static std::vector<PointLightShaderData> s_LightBuffer;
  static std::vector<ClusterData> s_ClusterBuffer;
};

#endif
