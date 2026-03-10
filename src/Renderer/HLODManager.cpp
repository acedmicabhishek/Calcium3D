#include "HLODManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <map>

std::vector<HLODProxy> HLODManager::s_Proxies;

struct GridKey {
  int x, y, z;
  bool operator<(const GridKey &other) const {
    if (x != other.x)
      return x < other.x;
    if (y != other.y)
      return y < other.y;
    return z < other.z;
  }
};

void HLODManager::BakeHLOD(Scene &scene, float clusterSize) {
  s_Proxies.clear();

  std::map<GridKey, std::vector<int>> clusters;
  auto &objects = scene.GetObjects();

  
  for (int i = 0; i < objects.size(); ++i) {
    const auto &obj = objects[i];
    if (!obj.isStatic || !obj.isActive || obj.mesh.vertices.empty())
      continue;

    GridKey key;
    key.x = static_cast<int>(std::floor(obj.position.x / clusterSize));
    key.y = static_cast<int>(std::floor(obj.position.y / clusterSize));
    key.z = static_cast<int>(std::floor(obj.position.z / clusterSize));

    clusters[key].push_back(i);
  }

  
  for (auto const &[key, clusterIndices] : clusters) {
    if (clusterIndices.size() < 2)
      continue; 

    HLODProxy proxy;
    proxy.originalObjectIndices = clusterIndices;

    std::vector<const GameObject *> clusterObjects;
    for (int idx : clusterIndices)
      clusterObjects.push_back(&objects[idx]);

    proxy.mesh = MergeMeshes(clusterObjects, scene);

    
    glm::vec3 minP(1e6f), maxP(-1e6f);
    for (const auto &v : proxy.mesh.vertices) {
      minP = glm::min(minP, v.position);
      maxP = glm::max(maxP, v.position);
    }

    proxy.center = (minP + maxP) * 0.5f;
    proxy.radius = glm::distance(minP, maxP) * 0.5f;
    proxy.threshold = clusterSize * 1.5f; 
    proxy.active = true;

    s_Proxies.push_back(std::move(proxy));
  }
}

void HLODManager::Clear() {
  for (auto &proxy : s_Proxies) {
    proxy.mesh.Delete();
  }
  s_Proxies.clear();
}

void HLODManager::Update(float dt) {
  
}

Mesh HLODManager::MergeMeshes(const std::vector<const GameObject *> &objects,
                              const Scene &scene) {
  std::vector<Vertex> mergedVertices;
  std::vector<GLuint> mergedIndices;
  std::vector<Texture>
      mergedTextures; 
                      

  if (!objects.empty()) {
    mergedTextures = objects[0]->mesh.textures;
  }

  for (const auto *obj : objects) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, obj->position);
    model = model * glm::mat4_cast(obj->rotation);
    model = glm::scale(model, obj->scale);

    GLuint baseIndex = static_cast<GLuint>(mergedVertices.size());

    for (const auto &v : obj->mesh.vertices) {
      Vertex mv = v;
      mv.position = glm::vec3(model * glm::vec4(v.position, 1.0f));
      mv.normal = glm::normalize(glm::vec3(glm::transpose(glm::inverse(model)) *
                                           glm::vec4(v.normal, 0.0f)));
      mergedVertices.push_back(mv);
    }

    for (const auto &idx : obj->mesh.indices) {
      mergedIndices.push_back(idx + baseIndex);
    }
  }

  return Mesh(mergedVertices, mergedIndices, mergedTextures);
}
