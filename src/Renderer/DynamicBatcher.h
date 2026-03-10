#ifndef DYNAMIC_BATCHER_H
#define DYNAMIC_BATCHER_H

#include "../Scene/Scene.h"
#include "Mesh.h"
#include "Shader.h"
#include <map>
#include <string>
#include <vector>

class DynamicBatcher {
public:
  static void DrawBatches(Scene &scene, Shader &shader, Camera &camera);

private:
  struct DynamicBatch {
    Material material;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
  };
};

#endif
