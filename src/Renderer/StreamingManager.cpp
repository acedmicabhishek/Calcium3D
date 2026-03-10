#include "StreamingManager.h"
#include "../Core/Logger.h"
#include "../ModelImport/ModelImporter.h"
#include "../Scene/Scene.h"
#include <glm/gtx/norm.hpp>

bool StreamingManager::s_EnableStreaming = true;
float StreamingManager::s_StreamingRadius = 100.0f;

void StreamingManager::Update(const glm::vec3 &cameraPos, Scene &scene) {
  if (!s_EnableStreaming)
    return;

  auto &objects = scene.GetObjects();
  float radiusSq = s_StreamingRadius * s_StreamingRadius;
  float hysteresisSq = radiusSq * 0.81f; 

  for (auto &obj : objects) {
    if (obj.modelPath.empty() || obj.meshType == MeshType::Cube ||
        obj.meshType == MeshType::Sphere || obj.meshType == MeshType::Plane)
      continue;
    if (obj.isStatic == false)
      continue; 

    float d2 = glm::distance2(cameraPos, obj.position);

    if (!obj.isStreamedOut && d2 > radiusSq) {
      
      obj.mesh.Delete();
      obj.isStreamedOut = true;
      Logger::AddLog("[Streaming] Unloaded: %s", obj.name.c_str());
    } else if (obj.isStreamedOut && d2 < hysteresisSq) {
      
      ImportResult res = ModelImporter::Import(obj.modelPath);
      if (res.success && obj.meshIndex >= 0 &&
          obj.meshIndex < (int)res.meshes.size()) {
        const auto &meshData = res.meshes[obj.meshIndex];
        obj.mesh = Mesh(meshData.vertices, meshData.indices, meshData.textures);
        obj.isStreamedOut = false;
        Logger::AddLog("[Streaming] Reloaded: %s", obj.name.c_str());
      }
    }
  }
}
