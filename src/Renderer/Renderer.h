#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

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

  static void RenderScene(Scene &scene, Camera &camera, Shader &shader,
                          float tilingFactor = 1.0f,
                          bool renderEditorObjects = true, float dt = 0.016f,
                          float time = 0.0f, int renderLayer = 0,
                          Camera *cullingCamera = nullptr);

  static void RenderHitboxes(Scene &scene, Camera &camera);

  static bool s_BackfaceCulling;
  static bool s_FrustumCulling;

private:
  static void Clear();
};

#endif
