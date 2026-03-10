#include "ShadowPass.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include "Camera.h"
#include "Frustum.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

ShadowPass::ShadowPass() { m_Name = "ShadowPass"; }

ShadowPass::~ShadowPass() {
  if (m_DirShadowFBO)
    glDeleteFramebuffers(1, &m_DirShadowFBO);
  if (m_DirShadowMap)
    glDeleteTextures(1, &m_DirShadowMap);

  if (m_PointShadowFBO)
    glDeleteFramebuffers(1, &m_PointShadowFBO);
  for (int i = 0; i < 4; i++) {
    if (m_PointShadowMaps[i])
      glDeleteTextures(1, &m_PointShadowMaps[i]);
  }
}

void ShadowPass::Init() {

  glGenFramebuffers(1, &m_DirShadowFBO);
  glGenTextures(1, &m_DirShadowMap);
  glBindTexture(GL_TEXTURE_2D, m_DirShadowMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_DirShadowRes,
               m_DirShadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, m_DirShadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         m_DirShadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &m_PointShadowFBO);
  for (int i = 0; i < 4; i++) {
    glGenTextures(1, &m_PointShadowMaps[i]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_PointShadowMaps[i]);
    for (unsigned int j = 0; j < 6; ++j) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT,
                   m_PointShadowRes, m_PointShadowRes, 0, GL_DEPTH_COMPONENT,
                   GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }
}

void ShadowPass::Execute(const RenderContext &context) {
  if (!context.scene)
    return;

  PROFILE_SCOPE("ShadowPass");
  GPU_PROFILE_SCOPE("ShadowPass");

  context.dirShadowMap = m_DirShadowMap;
  for (int i = 0; i < 4; i++)
    context.pointShadowCubemaps[i] = m_PointShadowMaps[i];

  if (!context.enableShadows && !context.enablePointShadows) {
    return;
  }

  glEnable(GL_DEPTH_TEST);

  if (context.enableShadows && context.sunEnabled) {
    PROFILE_SCOPE("DirShadows");
    glViewport(0, 0, m_DirShadowRes, m_DirShadowRes);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DirShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);

    glm::vec3 lightPos = context.sunPosition;
    if (glm::length(lightPos) < 1.0f)
      lightPos = glm::vec3(0, 100, 0);

    float orthoSize = 25.0f;
    glm::mat4 lightProjection =
        glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 100.0f);

    glm::vec3 target =
        context.camera ? context.camera->Position : glm::vec3(0.0f);
    glm::mat4 lightView = glm::lookAt(target + normalize(lightPos) * 30.0f,
                                      target, glm::vec3(0.0, 1.0, 0.0));

    context.lightSpaceMatrix = lightProjection * lightView;

    Shader &shadowShader = ResourceManager::GetShader("shadow");
    shadowShader.use();
    shadowShader.setMat4("lightSpaceMatrix", context.lightSpaceMatrix);

    auto &dirObjects = context.scene->GetObjects();
    for (size_t idx = 0; idx < dirObjects.size(); ++idx) {
      const auto &obj = dirObjects[idx];
      if (!obj.isActive || obj.meshType == MeshType::Camera)
        continue;

      glm::mat4 model = context.scene->GetGlobalTransform(idx);

      if (context.shadowCulling) {
        
        Frustum lightFrustum =
            Frustum::CreateFrustumFromCamera(context.lightSpaceMatrix);

        
        glm::vec3 corners[8] = {
            {obj.mesh.minAABB.x, obj.mesh.minAABB.y, obj.mesh.minAABB.z},
            {obj.mesh.maxAABB.x, obj.mesh.minAABB.y, obj.mesh.minAABB.z},
            {obj.mesh.minAABB.x, obj.mesh.maxAABB.y, obj.mesh.minAABB.z},
            {obj.mesh.maxAABB.x, obj.mesh.maxAABB.y, obj.mesh.minAABB.z},
            {obj.mesh.minAABB.x, obj.mesh.minAABB.y, obj.mesh.maxAABB.z},
            {obj.mesh.maxAABB.x, obj.mesh.minAABB.y, obj.mesh.maxAABB.z},
            {obj.mesh.minAABB.x, obj.mesh.maxAABB.y, obj.mesh.maxAABB.z},
            {obj.mesh.maxAABB.x, obj.mesh.maxAABB.y, obj.mesh.maxAABB.z}};

        glm::vec3 worldMin(1e30f), worldMax(-1e30f);
        glm::mat4 finalM =
            glm::scale(model, glm::vec3(context.globalTilingFactor));

        for (int c = 0; c < 8; c++) {
          glm::vec3 wc = glm::vec3(finalM * glm::vec4(corners[c], 1.0f));
          worldMin = glm::min(worldMin, wc);
          worldMax = glm::max(worldMax, wc);
        }

        if (!lightFrustum.IsOnFrustum(worldMin, worldMax))
          continue;
      }

      shadowShader.setMat4(
          "model", glm::scale(model, glm::vec3(context.globalTilingFactor)));
      obj.mesh.vao.Bind();
      glDrawElements(GL_TRIANGLES, obj.mesh.indices.size(), GL_UNSIGNED_INT, 0);
      obj.mesh.vao.Unbind();
    }
  }

  if (context.enablePointShadows) {
    PROFILE_SCOPE("PointShadows");
    glDisable(GL_CULL_FACE);

    Shader &pointShadowShader = ResourceManager::GetShader("point_shadow");

    int shadowCasters = 0;
    const auto &pointLights = context.scene->GetPointLights();

    for (const auto &light : pointLights) {
      if (!light.enabled || !light.castShadows)
        continue;
      if (shadowCasters >= 4)
        break;

      float far = context.pointShadowFarPlane;

      
      if (context.lightCulling && context.camera) {
        Frustum camFrustum = Frustum::CreateFrustumFromCamera(
            context.camera->GetProjectionMatrix() *
            context.camera->GetViewMatrix());

        
        if (!camFrustum.IsSphereOnFrustum(light.position, far)) {
          shadowCasters++; 
          continue;
        }
      }

      
      int targetRes = m_PointShadowRes;
      if (context.adaptiveShadowRes && context.camera) {
        float dist = glm::distance(light.position, context.camera->Position);
        if (dist > far * 0.7f)
          targetRes /= 4;
        else if (dist > far * 0.4f)
          targetRes /= 2;
      }
      targetRes = glm::max(targetRes, 32);

      
      if (targetRes != m_CurrentPointShadowRes[shadowCasters]) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_PointShadowMaps[shadowCasters]);
        for (unsigned int j = 0; j < 6; ++j) {
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0,
                       GL_DEPTH_COMPONENT, targetRes, targetRes, 0,
                       GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        m_CurrentPointShadowRes[shadowCasters] = targetRes;
      }

      glViewport(0, 0, targetRes, targetRes);
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           m_PointShadowMaps[shadowCasters], 0);
      glClear(GL_DEPTH_BUFFER_BIT);

      float aspect = 1.0f;
      float near = 0.5f;
      glm::mat4 shadowProj =
          glm::perspective(glm::radians(90.0f), aspect, near, far);

      glm::mat4 shadowTransforms[6] = {
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0)),
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(-1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0)),
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0, 1.0, 0.0),
                                   glm::vec3(0.0, 0.0, 1.0)),
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0, -1.0, 0.0),
                                   glm::vec3(0.0, 0.0, -1.0)),
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0, 0.0, 1.0),
                                   glm::vec3(0.0, -1.0, 0.0)),
          shadowProj * glm::lookAt(light.position,
                                   light.position + glm::vec3(0.0, 0.0, -1.0),
                                   glm::vec3(0.0, -1.0, 0.0))};

      pointShadowShader.use();
      for (int j = 0; j < 6; ++j)
        pointShadowShader.setMat4("shadowMatrices[" + std::to_string(j) + "]",
                                  shadowTransforms[j]);
      pointShadowShader.setVec3("lightPos", light.position);
      pointShadowShader.setFloat("far_plane", far);

      
      auto &ptObjects = context.scene->GetObjects();
      for (size_t idx = 0; idx < ptObjects.size(); ++idx) {
        const auto &obj = ptObjects[idx];
        if (!obj.isActive || obj.meshType == MeshType::Camera)
          continue;

        glm::mat4 model = context.scene->GetGlobalTransform(idx);
        glm::mat4 finalM =
            glm::scale(model, glm::vec3(context.globalTilingFactor));

        if (context.shadowCulling) {
          
          glm::vec3 objCenter = glm::vec3(
              finalM *
              glm::vec4((obj.mesh.minAABB + obj.mesh.maxAABB) * 0.5f, 1.0f));
          float objRadius =
              glm::length(obj.mesh.maxAABB - obj.mesh.minAABB) * 0.5f;
          
          float maxScale =
              glm::max(glm::length(glm::vec3(finalM[0])),
                       glm::max(glm::length(glm::vec3(finalM[1])),
                                glm::length(glm::vec3(finalM[2]))));
          objRadius *= maxScale;

          if (glm::distance(light.position, objCenter) > far + objRadius)
            continue;
        }

        pointShadowShader.setMat4("model", finalM);
        obj.mesh.vao.Bind();
        glDrawElements(GL_TRIANGLES, obj.mesh.indices.size(), GL_UNSIGNED_INT,
                       0);
        obj.mesh.vao.Unbind();
      }
      shadowCasters++;
    }
  }

  glDisable(GL_CULL_FACE);
  glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
  glViewport(0, 0, context.width, context.height);
}

void ShadowPass::Resize(int width, int height) {}
