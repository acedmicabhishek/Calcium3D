#include "UnderwaterPass.h"
#include "../../Core/ResourceManager.h"
#include "../Camera.h"
#include "../Scene/Scene.h"
#include "../Tools/Profiler/GpuProfiler.h"
#include "../Tools/Profiler/Profiler.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

UnderwaterPass::UnderwaterPass() { m_Name = "UnderwaterPass"; }

UnderwaterPass::~UnderwaterPass() {
  if (m_QuadVAO != 0) {
    glDeleteVertexArrays(1, &m_QuadVAO);
    glDeleteBuffers(1, &m_QuadVBO);
  }
  if (m_ResolveFBO != 0) {
    glDeleteFramebuffers(1, &m_ResolveFBO);
    glDeleteTextures(1, &m_ResolveColorObj);
    glDeleteTextures(1, &m_ResolveDepthObj);
  }
}

void UnderwaterPass::SetupQuad() {
  float quadVertices[] = {

      -1.0f, 1.0f, 0.0f, 0.0f,  1.0f, -1.0f, -1.0f, 0.0f,
      0.0f,  0.0f, 1.0f, -1.0f, 0.0f, 1.0f,  0.0f,

      -1.0f, 1.0f, 0.0f, 0.0f,  1.0f, 1.0f,  -1.0f, 0.0f,
      1.0f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f,  1.0f};

  glGenVertexArrays(1, &m_QuadVAO);
  glGenBuffers(1, &m_QuadVBO);
  glBindVertexArray(m_QuadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}

void UnderwaterPass::ResizeResolveFBO(int width, int height) {
  if (width <= 0 || height <= 0)
    return;
  if (m_ResolveFBO != 0) {
    glDeleteFramebuffers(1, &m_ResolveFBO);
    glDeleteTextures(1, &m_ResolveColorObj);
    glDeleteTextures(1, &m_ResolveDepthObj);
  }
  m_LastWidth = width;
  m_LastHeight = height;

  glGenFramebuffers(1, &m_ResolveFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_ResolveFBO);

  glGenTextures(1, &m_ResolveColorObj);
  glBindTexture(GL_TEXTURE_2D, m_ResolveColorObj);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         m_ResolveColorObj, 0);

  glGenTextures(1, &m_ResolveDepthObj);
  glBindTexture(GL_TEXTURE_2D, m_ResolveDepthObj);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,
               GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                         GL_TEXTURE_2D, m_ResolveDepthObj, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void UnderwaterPass::Init() { SetupQuad(); }

void UnderwaterPass::Execute(const RenderContext &context) {
  if (!context.scene || !context.camera || context.mainFBO == 0)
    return;

  PROFILE_SCOPE("UnderwaterPass");
  GPU_PROFILE_SCOPE("UnderwaterPass");

  float waterY = -1e9f;
  glm::vec3 waterColor = glm::vec3(0.0f, 0.4f, 0.8f);
  bool isUnderwater = false;

  auto &objects = context.scene->GetObjects();
  for (int i = 0; i < (int)objects.size(); ++i) {
    auto &obj = objects[i];
    if (obj.hasWater && obj.isActive) {

      glm::mat4 model = context.scene->GetGlobalTransform(i);
      glm::mat4 invModel = glm::inverse(model);

      glm::vec4 localCamPosVec =
          invModel * glm::vec4(context.camera->Position, 1.0f);
      glm::vec3 localCamPos = glm::vec3(localCamPosVec);

      bool insideXZ = (localCamPos.x >= -0.52f && localCamPos.x <= 0.52f &&
                       localCamPos.z >= -0.52f && localCamPos.z <= 0.52f);

      float surfaceLocal =
          obj.water.surfaceHeight / (obj.scale.y > 0.001f ? obj.scale.y : 1.0f);
      float bottomLocal = (obj.water.surfaceHeight - obj.water.depth) /
                          (obj.scale.y > 0.001f ? obj.scale.y : 1.0f);

      bool isInfinite = (obj.scale.x > 5000.0f || obj.scale.z > 5000.0f);

      if ((insideXZ || isInfinite) && localCamPos.y <= surfaceLocal &&
          localCamPos.y >= bottomLocal) {
        waterY = obj.position.y + obj.water.surfaceHeight;
        waterColor = obj.water.waterColor;
        isUnderwater = true;
        break;
      }
    }
  }

  if (!isUnderwater) {

    return;
  }

  if (context.width != m_LastWidth || context.height != m_LastHeight) {
    ResizeResolveFBO(context.width, context.height);
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, context.mainFBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolveFBO);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                    context.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBlitFramebuffer(0, 0, context.width, context.height, 0, 0, context.width,
                    context.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, context.mainFBO);
  glDisable(GL_DEPTH_TEST);

  if (!ResourceManager::HasShader("underwater")) {
    ResourceManager::LoadShader(
        "underwater", "../shaders/passes/postprocess/underwater.vert",
        "../shaders/passes/postprocess/underwater.frag");
  }
  Shader &shader = ResourceManager::GetShader("underwater");
  shader.use();

  for (auto &obj : objects) {
    if (obj.hasWater && obj.isActive) {
      glm::mat4 model = context.scene->GetGlobalTransform(&obj - &objects[0]);
      glm::mat4 invModel = glm::inverse(model);
      glm::vec3 localCam =
          glm::vec3(invModel * glm::vec4(context.camera->Position, 1.0f));

      float sH = obj.water.surfaceHeight;
      float sY = obj.scale.y > 0.001f ? obj.scale.y : 1.0f;
      float surfaceL = sH / sY;
      float bottomL = (sH - obj.water.depth) / sY;

      bool isInfinite = (obj.scale.x > 5000.0f || obj.scale.z > 5000.0f);
      bool insideXZ =
          isInfinite || (localCam.x >= -0.52f && localCam.x <= 0.52f &&
                         localCam.z >= -0.52f && localCam.z <= 0.52f);

      if (insideXZ && localCam.y <= surfaceL && localCam.y >= bottomL) {

        shader.setMat4("invModel", invModel);
        shader.setVec3("localBoxMin",
                       glm::vec3(isInfinite ? -1e6f : -0.5f, bottomL,
                                 isInfinite ? -1e6f : -0.5f));
        shader.setVec3("localBoxMax",
                       glm::vec3(isInfinite ? 1e6f : 0.5f, surfaceL,
                                 isInfinite ? 1e6f : 0.5f));
        shader.setFloat("liquidDensity", obj.water.liquidDensity);

        shader.setMat4("invView",
                       glm::inverse(context.camera->GetViewMatrix()));
        shader.setMat4("invProjection",
                       glm::inverse(context.camera->GetProjectionMatrix()));
        shader.setVec3("camPos", context.camera->Position);
        shader.setVec3("waterColor", obj.water.waterColor);
        shader.setFloat("time", (float)context.time);
        shader.setFloat("waterLevel", obj.position.y + obj.water.surfaceHeight);
        break;
      }
    }
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_ResolveColorObj);
  shader.setInt("screenTexture", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_ResolveDepthObj);
  shader.setInt("depthTexture", 1);

  glBindVertexArray(m_QuadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);
}

void UnderwaterPass::Resize(int width, int height) {
  ResizeResolveFBO(width, height);
}
