#include "LODGenerator.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <map>
#include <set>


void LODGenerator::SimplifyMesh(const std::vector<Vertex> &inVertices,
                                const std::vector<GLuint> &inIndices,
                                float targetRatio,
                                std::vector<Vertex> &outVertices,
                                std::vector<GLuint> &outIndices) {

  if (inIndices.size() < 30 || targetRatio >= 0.99f) {
    outVertices = inVertices;
    outIndices = inIndices;
    return;
  }

  int targetTriCount = (int)((inIndices.size() / 3) * targetRatio);
  if (targetTriCount < 4)
    targetTriCount = 4;

  std::vector<Vertex> verts = inVertices;
  std::vector<GLuint> inds = inIndices;

  
  
  
  

  
  glm::vec3 minV(1e10f), maxV(-1e10f);
  for (const auto &v : verts) {
    minV = glm::min(minV, v.position);
    maxV = glm::max(maxV, v.position);
  }

  
  glm::vec3 extent = maxV - minV;
  float maxExtent = std::max(extent.x, std::max(extent.y, extent.z));

  
  
  int gridSize = (int)(20.0f * std::sqrt(targetRatio));
  if (gridSize < 2)
    gridSize = 2;
  float cellSize = maxExtent / gridSize;

  std::map<int, GLuint>
      gridToVertexMap; 
                       
  std::map<int, int> gridCountMap; 
  std::vector<int> vertexRemap(verts.size(), -1);

  for (size_t i = 0; i < verts.size(); ++i) {
    glm::vec3 normalizedPos = (verts[i].position - minV) / cellSize;
    int gx = (int)normalizedPos.x;
    int gy = (int)normalizedPos.y;
    int gz = (int)normalizedPos.z;

    
    int cellIdx = gx + gy * gridSize + gz * gridSize * gridSize;

    if (gridToVertexMap.find(cellIdx) == gridToVertexMap.end()) {
      gridToVertexMap[cellIdx] = outVertices.size();
      gridCountMap[cellIdx] = 1;
      outVertices.push_back(verts[i]);
      vertexRemap[i] = outVertices.size() - 1;
    } else {
      vertexRemap[i] = gridToVertexMap[cellIdx];

      GLuint repIdx = gridToVertexMap[cellIdx];
      int count = ++gridCountMap[cellIdx];
      float weight = 1.0f / (float)count;
      float prevWeight = 1.0f - weight;

      outVertices[repIdx].position = outVertices[repIdx].position * prevWeight +
                                     verts[i].position * weight;
      outVertices[repIdx].normal = glm::normalize(
          outVertices[repIdx].normal * prevWeight + verts[i].normal * weight);
      outVertices[repIdx].color =
          outVertices[repIdx].color * prevWeight + verts[i].color * weight;
    }
  }

  
  for (size_t i = 0; i < inds.size(); i += 3) {
    GLuint v0 = vertexRemap[inds[i]];
    GLuint v1 = vertexRemap[inds[i + 1]];
    GLuint v2 = vertexRemap[inds[i + 2]];

    
    if (v0 != v1 && v1 != v2 && v2 != v0) {
      outIndices.push_back(v0);
      outIndices.push_back(v1);
      outIndices.push_back(v2);
    }
  }

  if (outIndices.empty()) {
    
    outVertices = inVertices;
    outIndices = inIndices;
  }
}
