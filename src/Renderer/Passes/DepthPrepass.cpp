#include "DepthPrepass.h"
#include "../Renderer.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "ResourceManager.h"

DepthPrepass::DepthPrepass() { m_Name = "DepthPrepass"; }

void DepthPrepass::Init() {}

void DepthPrepass::Execute(const RenderContext &context) {
  if (!context.zPrepass)
    return;

  PROFILE_SCOPE("DepthPrepass");
  GPU_PROFILE_SCOPE("DepthPrepass");

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 

  if (!ResourceManager::HasShader("depth_prepass")) {
    ResourceManager::LoadShader("depth_prepass",
                                "shaders/passes/geometry/depth_prepass.vert",
                                "shaders/passes/geometry/depth_prepass.frag");
  }

  Shader &shader = ResourceManager::GetShader("depth_prepass");
  shader.use();
  shader.setMat4("view", context.camera->GetViewMatrix());
  shader.setMat4("projection", context.camera->GetProjectionMatrix());

  if (context.scene) {
    Renderer::RenderScene(
        *context.scene, *context.camera, shader, context.globalTilingFactor,
        context.renderEditorObjects, context.deltaTime, context.time,
        1, 
        context.cullingCamera, context.objCulling, context.backfaceCulling,
        true,  
        false, 
        context.autoLOD, false, context.staticBatching,
        context.dynamicBatching);
  }

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 

  
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
}
