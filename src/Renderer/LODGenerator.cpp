#include "LODGenerator.h"
#include "../Core/Logger.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <map>
#include <set>

struct Quadric {
  double a2, ab, ac, ad;
  double b2, bc, bd;
  double c2, cd;
  double d2;

  Quadric()
      : a2(0), ab(0), ac(0), ad(0), b2(0), bc(0), bd(0), c2(0), cd(0), d2(0) {}

  void AddPlane(double a, double b, double c, double d) {
    a2 += a * a;
    ab += a * b;
    ac += a * c;
    ad += a * d;
    b2 += b * b;
    bc += b * c;
    bd += b * d;
    c2 += c * c;
    cd += c * d;
    d2 += d * d;
  }

  void operator+=(const Quadric &other) {
    a2 += other.a2;
    ab += other.ab;
    ac += other.ac;
    ad += other.ad;
    b2 += other.b2;
    bc += other.bc;
    bd += other.bd;
    c2 += other.c2;
    cd += other.cd;
    d2 += other.d2;
  }

  
  glm::vec3 Solve(const glm::vec3 &fallback) const {
    
    
    
    double det = a2 * (b2 * c2 - bc * bc) - ab * (ab * c2 - ac * bc) +
                 ac * (ab * bc - ac * b2);

    if (std::abs(det) < 1e-8)
      return fallback;

    double idet = 1.0 / det;
    glm::vec3 result;
    result.x = (float)(idet * ((-ad) * (b2 * c2 - bc * bc) -
                               ab * ((-bd) * c2 - cd * bc) +
                               ac * ((-bd) * bc - cd * b2)));
    result.y = (float)(idet * (a2 * ((-bd) * c2 - cd * bc) -
                               (-ad) * (ab * c2 - ac * bc) +
                               ac * (ab * cd - (-ad) * ac)));
    result.z = (float)(idet * (a2 * (b2 * cd - (-bd) * bc) -
                               ab * (ab * cd - (-ad) * bc) +
                               (-ad) * (ab * bc - ac * b2)));

    return result;
  }

  
  double GetError(const glm::vec3 &p) const {
    double x = p.x, y = p.y, z = p.z;
    return a2 * x * x + 2 * ab * x * y + 2 * ac * x * z + 2 * ad * x +
           b2 * y * y + 2 * bc * y * z + 2 * bd * y + c2 * z * z + 2 * cd * z +
           d2;
  }
};

void LODGenerator::SimplifyMesh(const std::vector<Vertex> &inVertices,
                                const std::vector<GLuint> &inIndices,
                                float targetRatio,
                                std::vector<Vertex> &outVertices,
                                std::vector<GLuint> &outIndices) {

  if (inIndices.size() < 40 || targetRatio >= 0.99f) {
    outVertices = inVertices;
    outIndices = inIndices;
    return;
  }

  
  glm::vec3 minV(1e10f), maxV(-1e10f);
  for (const auto &v : inVertices) {
    minV = glm::min(minV, v.position);
    maxV = glm::max(maxV, v.position);
  }

  glm::vec3 extent = maxV - minV;
  float maxExtent = std::max(extent.x, std::max(extent.y, extent.z));

  
  
  int gridSize = (int)(96.0f * std::pow(targetRatio, 0.45f));
  if (gridSize < 4)
    gridSize = 4;
  float cellSize = maxExtent / (float)gridSize;

  struct CellData {
    Quadric q;
    std::vector<size_t> originalVertices;
    glm::vec3 weightedNormal = glm::vec3(0.0f);
    glm::vec3 weightedColor = glm::vec3(0.0f);
    glm::vec2 weightedUV = glm::vec2(0.0f);
    float weight = 0.0f;
  };

  std::map<int, CellData> grid;
  std::vector<int> vertexToCell(inVertices.size(), -1);

  
  for (size_t i = 0; i < inVertices.size(); ++i) {
    glm::vec3 relPos = (inVertices[i].position - minV) / cellSize;
    int gx = std::clamp((int)relPos.x, 0, gridSize - 1);
    int gy = std::clamp((int)relPos.y, 0, gridSize - 1);
    int gz = std::clamp((int)relPos.z, 0, gridSize - 1);
    int cellIdx = gx + gy * gridSize + gz * gridSize * gridSize;

    vertexToCell[i] = cellIdx;
    auto &cell = grid[cellIdx];
    cell.originalVertices.push_back(i);
    cell.weightedNormal += inVertices[i].normal;
    cell.weightedColor += inVertices[i].color;
    cell.weightedUV += inVertices[i].texUV;
    cell.weight += 1.0f;
  }

  
  for (size_t i = 0; i < inIndices.size(); i += 3) {
    const Vertex &v0 = inVertices[inIndices[i]];
    const Vertex &v1 = inVertices[inIndices[i + 1]];
    const Vertex &v2 = inVertices[inIndices[i + 2]];

    glm::vec3 edge1 = v1.position - v0.position;
    glm::vec3 edge2 = v2.position - v0.position;
    glm::vec3 cross = glm::cross(edge1, edge2);
    float area = glm::length(cross) * 0.5f;
    if (area < 1e-9f)
      continue;

    glm::vec3 normal = cross / (2.0f * area);
    float d = -glm::dot(normal, v0.position);

    Quadric triQ;
    triQ.AddPlane(normal.x, normal.y, normal.z, d);
    
    triQ.a2 *= area;
    triQ.ab *= area;
    triQ.ac *= area;
    triQ.ad *= area;
    triQ.b2 *= area;
    triQ.bc *= area;
    triQ.bd *= area;
    triQ.c2 *= area;
    triQ.cd *= area;
    triQ.d2 *= area;

    int c0 = vertexToCell[inIndices[i]];
    int c1 = vertexToCell[inIndices[i + 1]];
    int c2 = vertexToCell[inIndices[i + 2]];

    grid[c0].q += triQ;
    if (c0 != c1)
      grid[c1].q += triQ;
    if (c0 != c2 && c1 != c2)
      grid[c2].q += triQ;
  }

  
  std::map<int, GLuint> cellToOutIdx;
  for (auto &pair : grid) {
    int cellIdx = pair.first;
    auto &cell = pair.second;
    Vertex outV;

    
    glm::vec3 centroid(0.0f);
    for (size_t vIdx : cell.originalVertices)
      centroid += inVertices[vIdx].position;
    centroid /= (float)cell.originalVertices.size();

    
    glm::vec3 qemPos = cell.q.Solve(centroid);

    
    glm::vec3 cellMin =
        minV + glm::vec3(cellIdx % gridSize, (cellIdx / gridSize) % gridSize,
                         cellIdx / (gridSize * gridSize)) *
                   cellSize;
    glm::vec3 cellMax = cellMin + glm::vec3(cellSize);
    qemPos = glm::clamp(qemPos, cellMin, cellMax);

    
    double minErr = cell.q.GetError(qemPos);
    glm::vec3 bestPos = qemPos;
    size_t bestSourceIdx = cell.originalVertices[0];

    for (size_t vIdx : cell.originalVertices) {
      double err = cell.q.GetError(inVertices[vIdx].position);
      if (err < minErr) {
        minErr = err;
        bestPos = inVertices[vIdx].position;
        bestSourceIdx = vIdx;
      }
    }

    outV.position = bestPos;

    
    
    
    outV.normal = inVertices[bestSourceIdx].normal;
    outV.color = inVertices[bestSourceIdx].color;
    outV.texUV = inVertices[bestSourceIdx].texUV;

    cellToOutIdx[cellIdx] = outVertices.size();
    outVertices.push_back(outV);
  }

  
  for (size_t i = 0; i < inIndices.size(); i += 3) {
    GLuint v0 = cellToOutIdx[vertexToCell[inIndices[i]]];
    GLuint v1 = cellToOutIdx[vertexToCell[inIndices[i + 1]]];
    GLuint v2 = cellToOutIdx[vertexToCell[inIndices[i + 2]]];

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
