#ifndef HLOD_MANAGER_H
#define HLOD_MANAGER_H

#include "Mesh.h"
#include "Scene.h"
#include <memory>
#include <vector>

struct HLODProxy {
  Mesh mesh;
  glm::vec3 center;
  float radius;
  float threshold = 100.0f; 
  bool active = false;
  std::vector<int> originalObjectIndices;
};

class HLODManager {
public:
  static void BakeHLOD(Scene &scene, float clusterSize = 50.0f);
  static void Clear();

  static const std::vector<HLODProxy> &GetProxies() { return s_Proxies; }
  static void Update(float dt);

private:
  static std::vector<HLODProxy> s_Proxies;
  static Mesh MergeMeshes(const std::vector<const GameObject *> &objects,
                          const Scene &scene);
};

#endif
