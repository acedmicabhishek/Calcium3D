#ifndef STATIC_BATCHER_H
#define STATIC_BATCHER_H

#include "../Scene/Scene.h"
#include "Mesh.h"
#include "Shader.h"
#include <map>
#include <string>
#include <vector>

class StaticBatcher {
public:
  static void Bake(Scene &scene);
  static void DrawBatches(Shader &shader, Camera &camera);
  static void Clear();
  static bool HasBatches();

private:
  struct Batch {
    Mesh *combinedMesh = nullptr;
    Material material;
    std::vector<int> originalObjectIndices; 
  };

  
  static std::map<std::string, Batch> s_Batches;
};

#endif
