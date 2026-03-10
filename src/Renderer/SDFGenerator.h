#ifndef SDFGENERATOR_H
#define SDFGENERATOR_H

#include "Mesh.h"
#include <vector>

struct SDFVolume {
  unsigned int textureID;
  glm::vec3 minAABB;
  glm::vec3 maxAABB;
  int resolution;
};

class SDFGenerator {
public:
  static SDFVolume GenerateSDF(const Mesh &mesh, int resolution = 32);
  static float CalculateDistanceToMesh(const glm::vec3 &samplePoint,
                                       const Mesh &mesh);
};

#endif
