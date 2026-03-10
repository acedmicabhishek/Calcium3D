#include "TransparencyPass.h"
#include "../Physics/PhysicsEngine.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "2dCloud.h"
#include "Camera.h"
#include "Frustum.h"
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

  Frustum frustum = Frustum::CreateFrustumFromCamera(
      context.camera->GetProjectionMatrix() * context.camera->GetViewMatrix());

  std::vector<AABB> occluders;
  if (Renderer::s_EnableOcclusionCulling && context.scene) {
    for (const auto &obj : context.scene->GetObjects()) {
      if (obj.isActive && obj.isOccluder) {
        occluders.push_back(PhysicsEngine::GetTransformedAABB(
            obj.collider, obj.position, obj.rotation, obj.scale));
      }
    }
  }

  auto renderClouds = [&](const RenderContext &ctx) {
    if (ctx.showClouds) {
      if (ctx.cloudMode == 0 && ctx.cloud2d) {
        Shader &cloud2dShader = ResourceManager::GetShader("cloud2d");
        ctx.cloud2d->cloudCover = ctx.cloudCover;
        ctx.cloud2d->density = ctx.cloudDensity;

        bool useVRS = Renderer::s_VRS && !Renderer::s_VRSExcludeClouds;
        cloud2dShader.use();
        cloud2dShader.setInt("vrsMode", useVRS ? 3 : 0);
        cloud2dShader.setBool("debugVRS", Renderer::s_VisualizeVRS);

        glm::mat4 cloud2dModel = glm::mat4(1.0f);
        cloud2dModel = glm::translate(
            cloud2dModel, glm::vec3(ctx.camera->Position.x, ctx.cloudHeight,
                                    ctx.camera->Position.z));
        cloud2dModel = glm::rotate(cloud2dModel, glm::radians(-90.0f),
                                   glm::vec3(1.0f, 0.0f, 0.0f));
        float cloudScale = ctx.camera->farPlane * 10.0f;
        cloud2dModel = glm::scale(cloud2dModel, glm::vec3(cloudScale));

        bool isCulled = false;
        if (Renderer::s_ObjFrustumCulling) {
          glm::vec3 cCenter(ctx.camera->Position.x, ctx.cloudHeight,
                            ctx.camera->Position.z);
          float radius = cloudScale * 1.414f;
          if (!frustum.IsSphereOnFrustum(cCenter, radius)) {
            isCulled = true;
          }
        }

        if (!isCulled) {
          GLboolean cullWasEnabled = glIsEnabled(GL_CULL_FACE);
          glDisable(GL_CULL_FACE);
          ctx.cloud2d->Draw(cloud2dShader, *ctx.camera, cloud2dModel);
          if (cullWasEnabled)
            glEnable(GL_CULL_FACE);
          else
            glDisable(GL_CULL_FACE);
        }
      } else if (ctx.cloudMode == 1 && ctx.volCloud) {
        Shader &volCloudShader = ResourceManager::GetShader("volumetric_cloud");
        volCloudShader.setFloat("cloudCover", ctx.cloudCover);
        volCloudShader.setInt("vrsMode", Renderer::s_VRS ? 3 : 0);
        volCloudShader.setBool("debugVRS", Renderer::s_VisualizeVRS);

        bool isCulled = false;
        if (Renderer::s_ObjFrustumCulling) {
          glm::vec3 cCenter(ctx.camera->Position.x, ctx.cloudHeight,
                            ctx.camera->Position.z);
          float radius = ctx.camera->farPlane;
          if (!frustum.IsSphereOnFrustum(cCenter, radius)) {
            isCulled = true;
          }
        }

        if (!isCulled) {
          glDisable(GL_DEPTH_TEST);
          ctx.volCloud->Draw(volCloudShader, *ctx.camera, ctx.cloudHeight,
                             ctx.camera->farPlane);
          glEnable(GL_DEPTH_TEST);
        }
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
        glm::mat4 model = context.scene->GetGlobalTransform(i);

        bool isCulled = false;
        if (Renderer::s_ObjFrustumCulling) {
          glm::vec3 minP = obj.mesh.minAABB;
          glm::vec3 maxP = obj.mesh.maxAABB;
          glm::vec3 corners[8] = {
              {minP.x, minP.y, minP.z}, {maxP.x, minP.y, minP.z},
              {minP.x, maxP.y, minP.z}, {maxP.x, maxP.y, minP.z},
              {minP.x, minP.y, maxP.z}, {maxP.x, minP.y, maxP.z},
              {minP.x, maxP.y, maxP.z}, {maxP.x, maxP.y, maxP.z}};

          glm::vec3 worldMin(1e30f);
          glm::vec3 worldMax(-1e30f);
          for (int c = 0; c < 8; c++) {
            glm::vec3 worldCorner =
                glm::vec3(model * glm::vec4(corners[c], 1.0f));
            worldMin = glm::min(worldMin, worldCorner);
            worldMax = glm::max(worldMax, worldCorner);
          }
          if (!frustum.IsOnFrustum(worldMin, worldMax)) {
            isCulled = true;
          }
        }

        if (!isCulled && Renderer::s_EnableOcclusionCulling &&
            !obj.isOccluder) {
          AABB worldAABB = PhysicsEngine::GetTransformedAABB(
              obj.collider, obj.position, obj.rotation, obj.scale);
          for (const auto &occ : occluders) {
            glm::vec3 toOcc =
                glm::normalize(occ.min + (occ.max - occ.min) * 0.5f -
                               context.camera->Position);
            glm::vec3 toObj = glm::normalize(
                worldAABB.min + (worldAABB.max - worldAABB.min) * 0.5f -
                context.camera->Position);
            float dot = glm::dot(toOcc, toObj);
            if (dot > 0.99f) {
              float distToOcc = glm::length(occ.min - context.camera->Position);
              float distToObj =
                  glm::length(worldAABB.min - context.camera->Position);
              if (distToObj > distToOcc + glm::length(occ.max - occ.min)) {
                isCulled = true;
                break;
              }
            }
          }
        }

        if (isCulled)
          continue;

        if (Renderer::s_BackfaceCulling) {
          float det = glm::determinant(glm::mat3(model));
          glEnable(GL_CULL_FACE);
          glFrontFace(det < 0 ? GL_CW : GL_CCW);
        }

        waterShader.use();
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

        if (Renderer::s_BackfaceCulling) {
          glFrontFace(GL_CCW);
        }
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
