#ifndef LOD_GENERATOR_H
#define LOD_GENERATOR_H

#include "Mesh.h"
#include <vector>

class LODGenerator {
public:
  static void SimplifyMesh(const std::vector<Vertex> &inVertices,
                           const std::vector<GLuint> &inIndices,
                           float targetRatio, std::vector<Vertex> &outVertices,
                           std::vector<GLuint> &outIndices);
};

#endif
