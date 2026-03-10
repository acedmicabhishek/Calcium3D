#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Shader.h"

class Renderer {
public:
  static void Init();
  static void Shutdown();

  static void BeginScene(Camera &camera,
                         const glm::vec4 &clearColor = glm::vec4(0.1f, 0.1f,
                                                                 0.1f, 1.0f));
  static void EndScene();

  static void RenderMesh(Mesh &mesh, Shader &shader, const glm::vec3 &position,
                         const glm::quat &rotation, const glm::vec3 &scale);

  static void RenderScene(
      Scene &scene, Camera &camera, Shader &shader, float tilingFactor = 1.0f,
      bool renderEditorObjects = true, float dt = 0.016f, float time = 0.0f,
      int renderLayer = 0, Camera *cullingCamera = nullptr,
      bool useObjCulling = true, bool useBackfaceCulling = true,
      bool useMaterialOptimisation = false, bool visualizeCulling = false,
      bool useAutoLOD = true, bool useZPrepass = false,
      bool useStaticBatching = false, bool useDynamicBatching = false);

  static void RenderHitboxes(Scene &scene, Camera &camera);

  static bool s_BackfaceCulling;
  static bool s_ObjFrustumCulling;
  static bool s_LightFrustumCulling;
  static bool s_ShadowFrustumCulling;
  static bool s_MaterialOptimisation;
  static bool s_ZPrepass;
  static bool s_ShowCulledAsWireframe;
  static bool s_VRS;
  static bool s_VRSExcludeClouds;
  static bool s_VRSExcludeWater;
  static bool s_VRSExcludeModels;
  static bool s_EnableSDFShadows;
  static bool s_EnableHLOD;
  static bool s_EnableOcclusionCulling;
  static bool s_VisualizeZPrepass;
  static bool s_VisualizeVRS;
  static bool s_AdaptiveShadowRes;
  static bool s_StaticBatching;
  static bool s_DynamicBatching;
  static bool s_ClusteredShading;
  static bool s_AutoLOD;
  static float s_LODDistances[4];
  static bool s_LODEnabled[4];

  static int s_MaxFPS;
  static bool s_LowLatencyMode;

private:
  static void Clear();
};

#endif
