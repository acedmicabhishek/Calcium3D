#include "TransparencyPass.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "2dCloud.h"
#include "Camera.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "SSRPass.h"
#include "Scene.h"
#include "VolumetricCloud.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TransparencyPass::TransparencyPass() { m_Name = "TransparencyPass"; }

void TransparencyPass::Init() {}

void TransparencyPass::Execute(const RenderContext &context) {
  PROFILE_SCOPE("TransparencyPass");
  GPU_PROFILE_SCOPE("TransparencyPass");

  if (context.mainFBO != 0) {
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT1};
    glDrawBuffers(1, attachments);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint restoreAttachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, restoreAttachments);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  auto renderClouds = [&](const RenderContext &ctx) {
    if (ctx.showClouds) {
      if (ctx.cloudMode == 0 && ctx.cloud2d) {
        Shader &cloud2dShader = ResourceManager::GetShader("cloud2d");
        ctx.cloud2d->cloudCover = ctx.cloudCover;
        ctx.cloud2d->density = ctx.cloudDensity;

        bool useVRS = Renderer::s_VRS && !Renderer::s_VRSExcludeClouds;
        cloud2dShader.use();
        cloud2dShader.setInt("vrsMode",
                             useVRS ? 3 : 0); 
        cloud2dShader.setBool("debugVRS", Renderer::s_VisualizeVRS);

        glm::mat4 cloud2dModel = glm::mat4(1.0f);
        cloud2dModel = glm::translate(
            cloud2dModel, glm::vec3(ctx.camera->Position.x, ctx.cloudHeight,
                                    ctx.camera->Position.z));
        cloud2dModel = glm::rotate(cloud2dModel, glm::radians(-90.0f),
                                   glm::vec3(1.0f, 0.0f, 0.0f));
        cloud2dModel =
            glm::scale(cloud2dModel, glm::vec3(ctx.camera->farPlane * 10.0f));

        GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
        glDisable(GL_CULL_FACE);
        ctx.cloud2d->Draw(cloud2dShader, *ctx.camera, cloud2dModel);
        if (cullWasEnabled)
          glEnable(GL_CULL_FACE);
        else
          glDisable(GL_CULL_FACE);
      } else if (ctx.cloudMode == 1 && ctx.volCloud) {
        Shader &volCloudShader = ResourceManager::GetShader("volumetric_cloud");
        volCloudShader.setFloat("cloudCover", ctx.cloudCover);
        volCloudShader.setInt(
            "vrsMode",
            Renderer::s_VRS ? 3 : 0); 
        volCloudShader.setBool("debugVRS", Renderer::s_VisualizeVRS);
        glDisable(GL_DEPTH_TEST);
        ctx.volCloud->Draw(volCloudShader, *ctx.camera, ctx.cloudHeight,
                           ctx.camera->farPlane);
        glEnable(GL_DEPTH_TEST);
      }
    }
  };

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  bool cameraIsAboveClouds = context.camera->Position.y > context.cloudHeight;

  if (!cameraIsAboveClouds) {
    renderClouds(context);
  }

  if (context.scene && context.camera) {
    Shader &waterShader = ResourceManager::GetShader("water");
    auto &objects = context.scene->GetObjects();
    for (int i = 0; i < objects.size(); ++i) {
      auto &obj = objects[i];
      if (obj.hasWater && obj.isActive) {
        waterShader.use();
        glm::mat4 model = context.scene->GetGlobalTransform(i);
        waterShader.setMat4("model", model);
        waterShader.setMat4("view", context.camera->GetViewMatrix());
        waterShader.setMat4("projection",
                            context.camera->GetProjectionMatrix());
        waterShader.setFloat("time", context.time);
        waterShader.setVec3("viewPos", context.camera->Position);
        waterShader.setFloat("waveSpeed", obj.water.waveSpeed);
        waterShader.setFloat("waveStrength", obj.water.waveStrength);
        waterShader.setFloat("shininess", obj.water.shininess);
        waterShader.setVec3("waterColor", obj.water.waterColor);
        waterShader.setInt("waveSystem", obj.water.waveSystem);
        waterShader.setFloat("tiling", obj.water.tiling);
        waterShader.setFloat("surfaceHeight", obj.water.surfaceHeight);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, SSRPass::GetGeometryDepth());
        waterShader.setInt("depthTexture", 5);
        waterShader.setMat4("invView",
                            glm::inverse(context.camera->GetViewMatrix()));
        waterShader.setMat4(
            "invProjection",
            glm::inverse(context.camera->GetProjectionMatrix()));
        waterShader.setVec2(
            "screenResolution",
            glm::vec2((float)context.width, (float)context.height));

        int vrsMode = 0;
        if (Renderer::s_VRS && !Renderer::s_VRSExcludeWater) {
          float dist =
              glm::distance(context.camera->Position, glm::vec3(model[3]));
          if (dist > 60.0f)
            vrsMode = 3;
          else if (dist > 35.0f)
            vrsMode = 2;
          else if (dist > 15.0f)
            vrsMode = 1;
        }
        waterShader.setInt("vrsMode", vrsMode);
        waterShader.setBool("debugVRS", Renderer::s_VisualizeVRS);

        obj.mesh.Draw(waterShader, *context.camera, model);
      }
    }
  }

  if (cameraIsAboveClouds) {
    renderClouds(context);
  }

  if (context.scene && context.camera) {
    Shader &defaultShader = ResourceManager::GetShader("default");

    Renderer::RenderScene(
        *context.scene, *context.camera, defaultShader,
        context.globalTilingFactor, context.renderEditorObjects,
        context.deltaTime, context.time, 2, nullptr, context.objCulling, false,
        context.materialOptimisation, context.visualizeCulling, context.autoLOD,
        context.zPrepass, context.staticBatching, context.dynamicBatching);
  }

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}
